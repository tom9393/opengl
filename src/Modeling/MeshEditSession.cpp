#include "pch.h"
#include "Modeling/MeshEditSession.h"
#include "Asset/AssetManager.h"
#include "Asset/Mesh.h"

namespace Engine {

    bool MeshEditSession::Begin(Entity entity) {
        if (m_Active) {
            std::cout << "[MeshEditSession] A session is already active." << std::endl;
            return false;
        }
        if (!entity || !entity.HasComponent<MeshComponent>()) {
            std::cout << "[MeshEditSession] Entity has no mesh to edit." << std::endl;
            return false;
        }

        UUID handle = entity.GetComponent<MeshComponent>().MeshHandle;
        Shared<Engine::Mesh> src = AssetManager::GetAsset<Engine::Mesh>(handle);
        if (!src || src->GetVBOs().empty()) {
            std::cout << "[MeshEditSession] Source mesh is not loaded." << std::endl;
            return false;
        }

        const std::vector<float>& verts = src->GetVertices();
        const std::vector<unsigned int>& indices = src->GetIndices();
        const BufferLayout& layout = src->GetVBOs()[0]->GetLayout();

        int floatStride = (int)(layout.GetStride() / sizeof(float));
        if (floatStride <= 0 || verts.empty() || indices.empty()) {
            std::cout << "[MeshEditSession] Source mesh has no usable geometry." << std::endl;
            return false;
        }

        int posOff = -1, uvOff = -1, tanOff = -1, bitanOff = -1;
        for (const auto& e : layout.GetElements()) {
            int fo = (int)(e.Offset / sizeof(float));
            if (e.Name == "a_Position")      posOff = fo;
            else if (e.Name == "a_TexCoord") uvOff = fo;
            else if (e.Name == "a_Tangent")  tanOff = fo;
            else if (e.Name == "a_Bitangent") bitanOff = fo;
        }
        if (posOff < 0) {
            std::cout << "[MeshEditSession] Source layout has no a_Position." << std::endl;
            return false;
        }

        int vertexCount = (int)(verts.size() / floatStride);

        auto readVec3 = [&](int v, int off) {
            int b = v * floatStride + off;
            return glm::vec3(verts[b], verts[b + 1], verts[b + 2]);
        };
        auto readVec2 = [&](int v, int off) {
            int b = v * floatStride + off;
            return glm::vec2(verts[b], verts[b + 1]);
        };

        // Weld render vertices into topological vertices by quantized position.
        const float weldScale = 100000.0f; // ~1e-5 tolerance
        std::map<std::array<long long, 3>, int> weldMap;
        std::vector<glm::vec3> positions;
        std::vector<int> renderToWelded(vertexCount);

        m_UV.clear();
        m_Tangent.clear();
        m_Bitangent.clear();

        for (int v = 0; v < vertexCount; ++v) {
            glm::vec3 p = readVec3(v, posOff);
            std::array<long long, 3> key = {
                std::llround(p.x * weldScale),
                std::llround(p.y * weldScale),
                std::llround(p.z * weldScale)
            };

            auto it = weldMap.find(key);
            if (it == weldMap.end()) {
                int welded = (int)positions.size();
                weldMap[key] = welded;
                positions.push_back(p);
                m_UV.push_back(uvOff >= 0 ? readVec2(v, uvOff) : glm::vec2(0.0f));
                m_Tangent.push_back(tanOff >= 0 ? readVec3(v, tanOff) : glm::vec3(1.0f, 0.0f, 0.0f));
                m_Bitangent.push_back(bitanOff >= 0 ? readVec3(v, bitanOff) : glm::vec3(0.0f, 1.0f, 0.0f));
                renderToWelded[v] = welded;
            } else {
                renderToWelded[v] = it->second;
            }
        }

        // Each render triangle becomes a face on welded indices; drop degenerates.
        std::vector<std::vector<int>> faces;
        for (size_t t = 0; t + 2 < indices.size(); t += 3) {
            int a = renderToWelded[indices[t]];
            int b = renderToWelded[indices[t + 1]];
            int c = renderToWelded[indices[t + 2]];
            if (a != b && b != c && a != c)
                faces.push_back({ a, b, c });
        }

        if (!m_HalfEdge.Build(positions, faces)) {
            std::cout << "[MeshEditSession] Mesh is non-manifold; cannot enter edit mode." << std::endl;
            m_HalfEdge.Clear();
            m_UV.clear();
            m_Tangent.clear();
            m_Bitangent.clear();
            return false;
        }

        m_Entity = entity;
        m_OriginalMeshHandle = handle;
        m_EditMeshHandle = UUID();
        m_Active = true;

        entity.GetComponent<MeshComponent>().MeshHandle = m_EditMeshHandle;

        SyncToMesh();
        return true;
    }

    void MeshEditSession::SyncToMesh() {
        if (!m_Active)
            return;

        int vCount = (int)m_HalfEdge.Vertices().size();

        // Fan-triangulate every face into welded-index triangles.
        std::vector<unsigned int> indices;
        for (int f = 0; f < (int)m_HalfEdge.Faces().size(); ++f) {
            std::vector<int> fv = m_HalfEdge.FaceVertices(f);
            for (size_t k = 1; k + 1 < fv.size(); ++k) {
                indices.push_back((unsigned int)fv[0]);
                indices.push_back((unsigned int)fv[k]);
                indices.push_back((unsigned int)fv[k + 1]);
            }
        }

        // Recompute smooth (area-weighted) normals from the triangulation.
        std::vector<glm::vec3> normals(vCount, glm::vec3(0.0f));
        for (size_t t = 0; t + 2 < indices.size(); t += 3) {
            int a = indices[t], b = indices[t + 1], c = indices[t + 2];
            glm::vec3 fn = glm::cross(m_HalfEdge.Position(b) - m_HalfEdge.Position(a),
                                      m_HalfEdge.Position(c) - m_HalfEdge.Position(a));
            normals[a] += fn;
            normals[b] += fn;
            normals[c] += fn;
        }
        for (glm::vec3& n : normals) {
            float len = glm::length(n);
            n = len > 0.0f ? n / len : glm::vec3(0.0f, 1.0f, 0.0f);
        }

        // Interleave into the canonical 5-attribute layout the G-buffer shader expects.
        std::vector<float> vertices;
        vertices.reserve(vCount * 14);
        glm::vec3 minAABB(std::numeric_limits<float>::max());
        glm::vec3 maxAABB(std::numeric_limits<float>::lowest());
        for (int v = 0; v < vCount; ++v) {
            const glm::vec3& p = m_HalfEdge.Position(v);
            const glm::vec3& n = normals[v];
            const glm::vec2& uv = m_UV[v];
            const glm::vec3& t = m_Tangent[v];
            const glm::vec3& bt = m_Bitangent[v];
            vertices.insert(vertices.end(), {
                p.x, p.y, p.z, n.x, n.y, n.z, uv.x, uv.y,
                t.x, t.y, t.z, bt.x, bt.y, bt.z
            });
            minAABB = glm::min(minAABB, p);
            maxAABB = glm::max(maxAABB, p);
        }

        Shared<VertexArray> vao = VertexArray::Create();
        Shared<VertexBuffer> vbo = VertexBuffer::Create((float*)vertices.data(), vertices.size() * sizeof(float));
        vbo->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float3, "a_Tangent" },
            { ShaderDataType::Float3, "a_Bitangent" }
        });
        vao->AddVertexBuffer(vbo);

        Shared<IndexBuffer> ebo = IndexBuffer::Create(indices.data(), indices.size());
        vao->SetIndexBuffer(ebo);

        std::vector<Shared<VertexBuffer>> vbos = { vbo };
        Shared<Engine::Mesh> mesh = MakeShared<Engine::Mesh>(vertices, indices, vao, vbos, ebo);

        AABB aabb;
        aabb.Min = minAABB;
        aabb.Max = maxAABB;
        mesh->SetAABB(aabb);

        AssetMetadata metadata;
        metadata.Handle = m_EditMeshHandle;
        metadata.Type = AssetType::Mesh;
        metadata.IsMemoryAsset = true;
        metadata.Name = "EditMesh";

        Shared<Asset> asset = mesh;
        AssetManager::RegisterAsset(metadata, asset);
    }

    void MeshEditSession::End(bool commit) {
        if (!m_Active)
            return;

        if (!commit && m_Entity && m_Entity.HasComponent<MeshComponent>())
            m_Entity.GetComponent<MeshComponent>().MeshHandle = m_OriginalMeshHandle;

        m_Active = false;
        m_HalfEdge.Clear();
        m_UV.clear();
        m_Tangent.clear();
        m_Bitangent.clear();
        m_Entity = Entity{};
        m_OriginalMeshHandle = 0;
        m_EditMeshHandle = 0;
    }

    // Append carried attributes for each appended vertex, copied from its source vertex.
    void MeshEditSession::AppendCarriedAttributes(const std::vector<int>& sources) {
        for (int src : sources) {
            m_UV.push_back(m_UV[src]);
            m_Tangent.push_back(m_Tangent[src]);
            m_Bitangent.push_back(m_Bitangent[src]);
        }
    }

    bool MeshEditSession::ExtrudeFaces(const std::vector<int>& faces) {
        if (!m_Active)
            return false;

        std::vector<int> sources;
        if (!m_HalfEdge.ExtrudeFaces(faces, sources))
            return false;

        AppendCarriedAttributes(sources);
        SyncToMesh();
        return true;
    }

    bool MeshEditSession::SubdivideFaces(const std::vector<int>& faces) {
        if (!m_Active)
            return false;

        std::vector<int> sources;
        if (!m_HalfEdge.SubdivideFaces(faces, sources))
            return false;

        AppendCarriedAttributes(sources);
        SyncToMesh();
        return true;
    }

    bool MeshEditSession::DeleteFaces(const std::vector<int>& faces) {
        if (!m_Active)
            return false;

        std::vector<int> remap; // old vertex index -> new index, or -1 if removed
        if (!m_HalfEdge.DeleteFaces(faces, remap))
            return false;

        // Compact the carried attributes the same way (new index <= old, so forward-safe).
        int newCount = 0;
        for (int v : remap)
            if (v >= 0) newCount = std::max(newCount, v + 1);

        std::vector<glm::vec2> uv(newCount);
        std::vector<glm::vec3> tan(newCount);
        std::vector<glm::vec3> bit(newCount);
        for (int v = 0; v < (int)remap.size(); ++v) {
            if (remap[v] < 0)
                continue;
            uv[remap[v]] = m_UV[v];
            tan[remap[v]] = m_Tangent[v];
            bit[remap[v]] = m_Bitangent[v];
        }
        m_UV = std::move(uv);
        m_Tangent = std::move(tan);
        m_Bitangent = std::move(bit);

        SyncToMesh();
        return true;
    }

}
