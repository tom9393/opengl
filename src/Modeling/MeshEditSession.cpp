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

        // Each render triangle becomes a face on welded indices; drop degenerates. The
        // per-corner UVs are read from the original (pre-weld) render vertices so seams
        // survive into the corner UV layer built below.
        std::vector<std::vector<int>> faces;
        std::vector<std::vector<glm::vec2>> faceCornerUV;
        for (size_t t = 0; t + 2 < indices.size(); t += 3) {
            int ra = indices[t], rb = indices[t + 1], rc = indices[t + 2];
            int a = renderToWelded[ra];
            int b = renderToWelded[rb];
            int c = renderToWelded[rc];
            if (a != b && b != c && a != c) {
                faces.push_back({ a, b, c });
                faceCornerUV.push_back({
                    uvOff >= 0 ? readVec2(ra, uvOff) : glm::vec2(0.0f),
                    uvOff >= 0 ? readVec2(rb, uvOff) : glm::vec2(0.0f),
                    uvOff >= 0 ? readVec2(rc, uvOff) : glm::vec2(0.0f)
                });
            }
        }

        if (!m_HalfEdge.Build(positions, faces)) {
            std::cout << "[MeshEditSession] Mesh is non-manifold; cannot enter edit mode." << std::endl;
            m_HalfEdge.Clear();
            m_UV.clear();
            m_Tangent.clear();
            m_Bitangent.clear();
            m_UVVerts.clear();
            m_CornerUV.clear();
            return false;
        }

        BuildCornerUVs(faces, faceCornerUV);

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
        int faceCount = (int)m_HalfEdge.Faces().size();

        // Recompute smooth (area-weighted) normals per welded vertex from a fan
        // triangulation of every face.
        std::vector<glm::vec3> normals(vCount, glm::vec3(0.0f));
        for (int f = 0; f < faceCount; ++f) {
            std::vector<int> fv = m_HalfEdge.FaceVertices(f);
            for (size_t k = 1; k + 1 < fv.size(); ++k) {
                int a = fv[0], b = fv[k], c = fv[k + 1];
                glm::vec3 fn = glm::cross(m_HalfEdge.Position(b) - m_HalfEdge.Position(a),
                                          m_HalfEdge.Position(c) - m_HalfEdge.Position(a));
                normals[a] += fn;
                normals[b] += fn;
                normals[c] += fn;
            }
        }
        for (glm::vec3& n : normals) {
            float len = glm::length(n);
            n = len > 0.0f ? n / len : glm::vec3(0.0f, 1.0f, 0.0f);
        }

        // Emit one render vertex per (welded vertex, UV vert) pair so the mesh splits at
        // UV seams; position/normal/tangent come from the welded vertex, UV from the
        // per-corner layer. Faces are fan-triangulated by their corner half-edges.
        const int floatsPerVertex = 14;
        std::map<std::pair<int, int>, unsigned int> cornerToRender;
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        glm::vec3 minAABB(std::numeric_limits<float>::max());
        glm::vec3 maxAABB(std::numeric_limits<float>::lowest());

        auto emit = [&](int he) -> unsigned int {
            int v = m_HalfEdge.HalfEdges()[he].Origin;
            int uvIdx = m_CornerUV[he];
            std::pair<int, int> key{ v, uvIdx };
            auto it = cornerToRender.find(key);
            if (it != cornerToRender.end())
                return it->second;

            const glm::vec3& p = m_HalfEdge.Position(v);
            const glm::vec3& n = normals[v];
            glm::vec2 uv = (uvIdx >= 0 && uvIdx < (int)m_UVVerts.size()) ? m_UVVerts[uvIdx] : m_UV[v];
            const glm::vec3& t = m_Tangent[v];
            const glm::vec3& bt = m_Bitangent[v];

            unsigned int idx = (unsigned int)(vertices.size() / floatsPerVertex);
            vertices.insert(vertices.end(), {
                p.x, p.y, p.z, n.x, n.y, n.z, uv.x, uv.y,
                t.x, t.y, t.z, bt.x, bt.y, bt.z
            });
            minAABB = glm::min(minAABB, p);
            maxAABB = glm::max(maxAABB, p);
            cornerToRender[key] = idx;
            return idx;
        };

        for (int f = 0; f < faceCount; ++f) {
            std::vector<int> he = m_HalfEdge.FaceHalfEdges(f);
            for (size_t k = 1; k + 1 < he.size(); ++k) {
                indices.push_back(emit(he[0]));
                indices.push_back(emit(he[k]));
                indices.push_back(emit(he[k + 1]));
            }
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
        m_UVVerts.clear();
        m_CornerUV.clear();
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

        int oldHECount = (int)m_HalfEdge.HalfEdges().size();
        std::vector<int> sources;
        if (!m_HalfEdge.ExtrudeFaces(faces, sources))
            return false;

        AppendCarriedAttributes(sources);
        SeedNewCornerUVs(oldHECount);
        SyncToMesh();
        return true;
    }

    bool MeshEditSession::SubdivideFaces(const std::vector<int>& faces) {
        if (!m_Active)
            return false;

        int oldHECount = (int)m_HalfEdge.HalfEdges().size();
        std::vector<int> sources;
        if (!m_HalfEdge.SubdivideFaces(faces, sources))
            return false;

        AppendCarriedAttributes(sources);
        SeedNewCornerUVs(oldHECount);
        SyncToMesh();
        return true;
    }

    bool MeshEditSession::DeleteFaces(const std::vector<int>& faces) {
        if (!m_Active)
            return false;

        std::vector<int> remap;   // old vertex index    -> new index, or -1 if removed
        std::vector<int> heRemap; // old half-edge index -> new index, or -1 if removed
        if (!m_HalfEdge.DeleteFaces(faces, remap, heRemap))
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

        // Compact the per-corner UVs onto the new half-edge indexing; UV verts that lose
        // all their corners simply go unreferenced (the editor ignores them).
        std::vector<int> cornerUV((int)m_HalfEdge.HalfEdges().size(), -1);
        for (int he = 0; he < (int)heRemap.size(); ++he)
            if (heRemap[he] >= 0)
                cornerUV[heRemap[he]] = m_CornerUV[he];
        m_CornerUV = std::move(cornerUV);

        SyncToMesh();
        return true;
    }

    void MeshEditSession::BuildCornerUVs(const std::vector<std::vector<int>>& faces,
                                         const std::vector<std::vector<glm::vec2>>& faceCornerUV) {
        m_UVVerts.clear();
        m_CornerUV.assign(m_HalfEdge.HalfEdges().size(), -1);

        // A corner's UV vert is keyed by (welded vertex, quantized UV): same vertex with
        // the same UV across faces shares a vert (one island); a differing UV at the same
        // vertex splits into a second vert (a seam). The scale matches the position weld.
        const float uvScale = 100000.0f;
        std::map<std::array<long long, 3>, int> uvMap;

        for (int f = 0; f < (int)faces.size(); ++f) {
            std::vector<int> he = m_HalfEdge.FaceHalfEdges(f);
            for (int k = 0; k < (int)he.size() && k < (int)faces[f].size(); ++k) {
                int weldedV = faces[f][k];
                const glm::vec2& uv = faceCornerUV[f][k];
                std::array<long long, 3> key = {
                    weldedV,
                    std::llround(uv.x * uvScale),
                    std::llround(uv.y * uvScale)
                };

                auto it = uvMap.find(key);
                int idx;
                if (it == uvMap.end()) {
                    idx = (int)m_UVVerts.size();
                    m_UVVerts.push_back(uv);
                    uvMap[key] = idx;
                } else {
                    idx = it->second;
                }
                m_CornerUV[he[k]] = idx;
            }
        }
    }

    void MeshEditSession::SeedNewCornerUVs(int oldHalfEdgeCount) {
        int newHECount = (int)m_HalfEdge.HalfEdges().size();
        m_CornerUV.resize(newHECount, -1);

        // New interior corners are seeded from the per-vertex UV; corners at the same
        // vertex with the same seed share a UV vert so new faces stay connected in UV
        // space. Boundary half-edges keep -1.
        const float uvScale = 100000.0f;
        std::map<std::array<long long, 3>, int> uvMap;
        for (int he = oldHalfEdgeCount; he < newHECount; ++he) {
            if (m_HalfEdge.HalfEdges()[he].Face == HalfEdgeMesh::InvalidIndex)
                continue;

            int v = m_HalfEdge.HalfEdges()[he].Origin;
            const glm::vec2& uv = m_UV[v];
            std::array<long long, 3> key = {
                v,
                std::llround(uv.x * uvScale),
                std::llround(uv.y * uvScale)
            };

            auto it = uvMap.find(key);
            int idx;
            if (it == uvMap.end()) {
                idx = (int)m_UVVerts.size();
                m_UVVerts.push_back(uv);
                uvMap[key] = idx;
            } else {
                idx = it->second;
            }
            m_CornerUV[he] = idx;
        }
    }

}
