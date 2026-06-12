#include "PmSceneSerializer.h"
#include "AssetManager.h"
#include "Prefab.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "Scene/Entity.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <unordered_map>

namespace Engine
{
    static constexpr uint32_t PMSCENE_VERSION = 1;

    // Forward declarations
    static ParsedScene ExtractParsedScene(const Shared<Prefab>& prefab);
    static void WriteBinary(const ParsedScene& scene, const std::string& path);
    static std::optional<ParsedScene> ReadBinary(const std::string& path);

    template<typename T> static void Write(std::ostream& o, const T& v) { o.write(reinterpret_cast<const char*>(&v), sizeof(T)); }
    template<typename T> static void Read(std::istream& i, T& v) { i.read(reinterpret_cast<char*>(&v), sizeof(T)); }

    static void WriteString(std::ostream& o, const std::string& s)
    {
        uint32_t n = (uint32_t)s.size();
        Write(o, n);
        o.write(s.data(), n);
    }

    static std::string ReadString(std::istream& i)
    {
        uint32_t n = 0;
        Read(i, n);
        std::string s(n, '\0');
        if (n) i.read(s.data(), n);
        return s;
    }

    void PmSceneSerializer::Serialize(const Shared<Prefab>& prefab, const std::string& path)
    {
        ParsedScene scene = ExtractParsedScene(prefab);
        WriteBinary(scene, path);

        // ExtractMaterial malloc'd embedded-texture pixels purely to write them out
        for (ParsedMaterial& m : scene.Materials)
            for (auto& [type, data] : m.EmbeddedTextures)
                if (data.Pixels) { free(data.Pixels); data.Pixels = nullptr; }
    }

    std::optional<ParsedScene> PmSceneSerializer::Deserialize(const std::string& path)
    {
        return ReadBinary(path);
    }

    // Reverse a material into ParsedMaterial
    static ParsedMaterial ExtractMaterial(const Shared<Material>& mat, UUID handle)
    {
        ParsedMaterial pm;
        pm.Hash = (uint64_t)handle;
        pm.AlbedoTint = glm::vec4(1.0f);
        pm.Emission = 0.0f;
        pm.Roughness = 1.0f;
        pm.Metallic = 0.0f;

        auto scalar = [&](MaterialType t, float def) {
            auto it = mat->m_Properties.find(t);
            if (it != mat->m_Properties.end() && std::holds_alternative<float>(it->second))
                return std::get<float>(it->second);
            return def;
        };

        auto tintIt = mat->m_Properties.find(MaterialType::AlbedoTint);
        if (tintIt != mat->m_Properties.end() && std::holds_alternative<glm::vec4>(tintIt->second))
            pm.AlbedoTint = std::get<glm::vec4>(tintIt->second);

        pm.Emission = scalar(MaterialType::Emission, 0.0f);
        pm.Roughness = scalar(MaterialType::RoughnessMultiplier, 1.0f);
        pm.Metallic = scalar(MaterialType::MetallicMultiplier, 0.0f);

        for (const auto& [type, value] : mat->m_Properties)
        {
            if (!std::holds_alternative<Shared<Texture2D>>(value)) continue;
            Shared<Texture2D> tex = std::get<Shared<Texture2D>>(value);
            if (!tex) continue;

            // Embedded -> GPU readback, external -> disk
            UUID texHandle = AssetManager::GetHandleForAsset(std::static_pointer_cast<Asset>(tex));
            bool external = false;
            if (texHandle != 0)
            {
                const AssetMetadata& meta = AssetManager::GetMetadata(texHandle);
                if (!meta.IsMemoryAsset && !meta.FilePath.empty() && std::filesystem::exists(meta.FilePath))
                {
                    pm.TexturePaths[type] = meta.FilePath;
                    external = true;
                }
            }

            if (!external)
            {
                std::vector<unsigned char> pixels;
                tex->ReadPixels(pixels);

                TextureLoadData data;
                data.Width = (int)tex->GetWidth();
                data.Height = (int)tex->GetHeight();
                data.Channels = 4;
                data.Pixels = (unsigned char*)malloc(pixels.size());
                if (!pixels.empty()) std::memcpy(data.Pixels, pixels.data(), pixels.size());
                pm.EmbeddedTextures[type] = data;
            }
        }

        return pm;
    }

    // Reverse an entity subtree into ParsedSceneNodes, collecting materials into scene.Materials
    static ParsedSceneNode ExtractNode(Entity e, ParsedScene& scene, std::unordered_map<uint64_t, uint32_t>& materialIndex)
    {
        ParsedSceneNode node;
        node.Name = e.HasComponent<TagComponent>() ? e.GetComponent<TagComponent>().tag : "";

        auto& tc = e.GetComponent<TransformComponent>();
        node.Translation = tc.translation;
        node.Rotation = tc.rotation;
        node.Scale = tc.scale;

        if (e.HasComponent<LightComponent>())
        {
            auto& lc = e.GetComponent<LightComponent>();
            ParsedLight pl;
            pl.Type = lc.Type;
            pl.Color = lc.Color;
            pl.Intensity = lc.Intensity;
            pl.CastShadows = lc.CastShadows;
            pl.InnerCutOff = lc.InnerCutOff;
            pl.OuterCutOff = lc.OuterCutOff;
            node.Light = pl;
        }

        if (e.HasComponent<SceneCameraComponent>())
        {
            auto& cc = e.GetComponent<SceneCameraComponent>();
            auto& cam = cc.sceneCamera;
            ParsedCamera pc;
            pc.Orthographic = cam->GetProjectionType() == SceneCamera::ProjectionType::Orthographic;
            pc.VerticalFOV = cam->GetPerspectiveVerticalFOV();
            pc.OrthographicSize = cam->GetOrthographicSize();
            pc.NearClip = cam->GetNearClip();
            pc.FarClip = cam->GetFarClip();
            pc.Primary = cc.primary;
            node.Camera = pc;
        }

        if (e.HasComponent<MeshComponent>())
        {
            UUID meshHandle = e.GetComponent<MeshComponent>().MeshHandle;
            Shared<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
            if (mesh)
            {
                ParsedMesh pm;
                pm.Name = node.Name;
                pm.Hash = (uint64_t)meshHandle;
                pm.Vertices = mesh->GetVertices();
                pm.Indices = mesh->GetIndices();
                if (!mesh->GetVBOs().empty())
                    pm.Layout = mesh->GetVBOs()[0]->GetLayout();
                pm.MinExtents = mesh->GetAABB().Min;
                pm.MaxExtents = mesh->GetAABB().Max;
                pm.MaterialIndex = UINT32_MAX; // sentinel: no material (BuildPrefab skips it)

                if (e.HasComponent<MaterialComponent>())
                {
                    UUID matHandle = e.GetComponent<MaterialComponent>().MaterialHandle;
                    auto it = materialIndex.find((uint64_t)matHandle);
                    if (it != materialIndex.end())
                        pm.MaterialIndex = it->second;
                    else if (Shared<Material> mat = AssetManager::GetAsset<Material>(matHandle))
                    {
                        uint32_t idx = (uint32_t)scene.Materials.size();
                        scene.Materials.push_back(ExtractMaterial(mat, matHandle));
                        materialIndex[(uint64_t)matHandle] = idx;
                        pm.MaterialIndex = idx;
                    }
                }

                node.Meshes.push_back(std::move(pm));
            }
        }

        for (entt::entity child : e.GetChildren())
            node.Children.push_back(ExtractNode(Entity{ child, e.GetScene() }, scene, materialIndex));

        return node;
    }

    // Reverse a prefab into a ParsedScene
    static ParsedScene ExtractParsedScene(const Shared<Prefab>& prefab)
    {
        ParsedScene scene;
        scene.Directory = ""; // baked materials store full texture paths

        Entity root = prefab->FindRootEntity();
        if (!root) return scene;
        scene.Name = root.GetComponent<TagComponent>().tag;

        std::unordered_map<uint64_t, uint32_t> materialIndex; // handle -> index into scene.Materials
        scene.RootNode = ExtractNode(root, scene, materialIndex);
        return scene;
    }

    static void WriteBinary(const ParsedScene& scene, const std::string& path)
    {
        std::ofstream out(path, std::ios::binary);
        if (!out) {
            std::cerr << "Failed to open .pmscene for writing: " << path << std::endl;
            return;
        }

        out.write("PMSC", 4);
        uint32_t version = PMSCENE_VERSION;
        Write(out, version);
        WriteString(out, scene.Name);

        // Materials
        uint32_t matCount = (uint32_t)scene.Materials.size();
        Write(out, matCount);
        for (const ParsedMaterial& m : scene.Materials)
        {
            Write(out, m.Hash);
            WriteString(out, m.Name);
            Write(out, m.AlbedoTint);
            Write(out, m.Emission);
            Write(out, m.Roughness);
            Write(out, m.Metallic);

            uint32_t extCount = (uint32_t)m.TexturePaths.size();
            Write(out, extCount);
            for (const auto& [type, p] : m.TexturePaths) { int32_t t = (int32_t)type; Write(out, t); WriteString(out, p); }

            uint32_t embCount = (uint32_t)m.EmbeddedTextures.size();
            Write(out, embCount);
            for (const auto& [type, data] : m.EmbeddedTextures)
            {
                int32_t t = (int32_t)type;
                int32_t w = data.Width, h = data.Height, c = data.Channels;
                Write(out, t); Write(out, w); Write(out, h); Write(out, c);
                uint64_t bytes = (uint64_t)w * h * c;
                Write(out, bytes);
                if (bytes && data.Pixels) out.write(reinterpret_cast<const char*>(data.Pixels), bytes);
            }
        }

        // Mesh table (geometry deduped by hash; nodes reference it by index)
        // This is because multiple nodes may share same mesh
        std::vector<const ParsedMesh*> meshTable;
        std::unordered_map<uint64_t, uint32_t> meshIndex;
        std::function<void(const ParsedSceneNode&)> collect = [&](const ParsedSceneNode& n) {
            for (const ParsedMesh& pm : n.Meshes)
                if (meshIndex.find(pm.Hash) == meshIndex.end())
                {
                    meshIndex[pm.Hash] = (uint32_t)meshTable.size();
                    meshTable.push_back(&pm);
                }
            for (const auto& c : n.Children) collect(c);
        };

        collect(scene.RootNode);

        uint32_t meshCount = (uint32_t)meshTable.size();
        Write(out, meshCount);
        for (const ParsedMesh* pm : meshTable)
        {
            Write(out, pm->Hash);
            WriteString(out, pm->Name);
            Write(out, pm->MinExtents);
            Write(out, pm->MaxExtents);

            const auto& elems = pm->Layout.GetElements();
            uint32_t elemCount = (uint32_t)elems.size();
            Write(out, elemCount);
            for (const BufferElement& e : elems)
            {
                WriteString(out, e.Name);
                int32_t t = (int32_t)e.Type;
                uint8_t norm = e.Normalized ? 1 : 0;
                uint8_t en = e.Enabled ? 1 : 0;
                Write(out, t); Write(out, norm); Write(out, en);
            }

            uint64_t vcount = pm->Vertices.size();
            Write(out, vcount);
            if (vcount) out.write(reinterpret_cast<const char*>(pm->Vertices.data()), vcount * sizeof(float));

            uint64_t icount = pm->Indices.size();
            Write(out, icount);
            if (icount) out.write(reinterpret_cast<const char*>(pm->Indices.data()), icount * sizeof(unsigned int));
        }

        // Node tree
        std::function<void(const ParsedSceneNode&)> writeNode = [&](const ParsedSceneNode& n) {
            // Transform
            WriteString(out, n.Name);
            Write(out, n.Translation);
            Write(out, n.Rotation);
            Write(out, n.Scale);

            // Lights
            uint8_t hasLight = n.Light ? 1 : 0;
            Write(out, hasLight);
            if (n.Light)
            {
                int32_t lt = (int32_t)n.Light->Type;
                uint8_t cs = n.Light->CastShadows ? 1 : 0;
                Write(out, lt);
                Write(out, n.Light->Color);
                Write(out, n.Light->Intensity);
                Write(out, cs);
                Write(out, n.Light->InnerCutOff);
                Write(out, n.Light->OuterCutOff);
            }

            // Camera
            uint8_t hasCam = n.Camera ? 1 : 0;
            Write(out, hasCam);
            if (n.Camera)
            {
                uint8_t ortho = n.Camera->Orthographic ? 1 : 0;
                uint8_t prim = n.Camera->Primary ? 1 : 0;
                Write(out, ortho);
                Write(out, n.Camera->VerticalFOV);
                Write(out, n.Camera->OrthographicSize);
                Write(out, n.Camera->NearClip);
                Write(out, n.Camera->FarClip);
                Write(out, prim);
            }

            // Mesh IDs
            uint32_t nm = (uint32_t)n.Meshes.size();
            Write(out, nm);
            for (const ParsedMesh& pm : n.Meshes)
            {
                uint32_t ti = meshIndex[pm.Hash];
                Write(out, ti);
                Write(out, pm.MaterialIndex);
            }

            // Children
            uint32_t cc = (uint32_t)n.Children.size();
            Write(out, cc);
            for (const auto& c : n.Children) writeNode(c);
        };

        writeNode(scene.RootNode);
    }

    // Read a node subtree, resolving mesh references through meshTable
    static ParsedSceneNode ReadNode(std::istream& in, const std::vector<ParsedMesh>& meshTable)
    {
        ParsedSceneNode n;
        n.Name = ReadString(in);
        Read(in, n.Translation);
        Read(in, n.Rotation);
        Read(in, n.Scale);

        uint8_t hasLight; Read(in, hasLight);
        if (hasLight)
        {
            ParsedLight pl;
            int32_t lt; Read(in, lt); pl.Type = (LightComponent::LightType)lt;
            Read(in, pl.Color);
            Read(in, pl.Intensity);
            uint8_t cs; Read(in, cs); pl.CastShadows = cs != 0;
            Read(in, pl.InnerCutOff);
            Read(in, pl.OuterCutOff);
            n.Light = pl;
        }

        uint8_t hasCam; Read(in, hasCam);
        if (hasCam)
        {
            ParsedCamera pc;
            uint8_t ortho; Read(in, ortho); pc.Orthographic = ortho != 0;
            Read(in, pc.VerticalFOV);
            Read(in, pc.OrthographicSize);
            Read(in, pc.NearClip);
            Read(in, pc.FarClip);
            uint8_t prim; Read(in, prim); pc.Primary = prim != 0;
            n.Camera = pc;
        }

        uint32_t nm; Read(in, nm);
        for (uint32_t i = 0; i < nm; i++)
        {
            uint32_t ti, mi;
            Read(in, ti);
            Read(in, mi);
            ParsedMesh pm = meshTable[ti]; // copy geometry; pair with this usage's material
            pm.MaterialIndex = mi;
            n.Meshes.push_back(std::move(pm));
        }

        uint32_t cc; Read(in, cc);
        for (uint32_t i = 0; i < cc; i++) n.Children.push_back(ReadNode(in, meshTable));
        return n;
    }

    static std::optional<ParsedScene> ReadBinary(const std::string& path)
    {
        std::ifstream in(path, std::ios::binary);
        if (!in) return std::nullopt;

        char magic[4];
        in.read(magic, 4);
        if (std::strncmp(magic, "PMSC", 4) != 0) {
            std::cerr << "Not a .pmscene file: " << path << std::endl;
            return std::nullopt;
        }

        uint32_t version = 0;
        Read(in, version);

        ParsedScene scene;
        scene.Directory = "";
        scene.Name = ReadString(in);

        // Materials
        uint32_t matCount = 0;
        Read(in, matCount);
        scene.Materials.resize(matCount);
        for (uint32_t i = 0; i < matCount; i++)
        {
            ParsedMaterial& m = scene.Materials[i];
            Read(in, m.Hash);
            m.Name = ReadString(in);
            Read(in, m.AlbedoTint);
            Read(in, m.Emission);
            Read(in, m.Roughness);
            Read(in, m.Metallic);

            uint32_t extCount = 0;
            Read(in, extCount);
            for (uint32_t k = 0; k < extCount; k++)
            {
                int32_t t; Read(in, t);
                m.TexturePaths[(MaterialType)t] = ReadString(in);
            }

            uint32_t embCount = 0;
            Read(in, embCount);
            for (uint32_t k = 0; k < embCount; k++)
            {
                int32_t t, w, h, c;
                uint64_t bytes = 0;
                Read(in, t); Read(in, w); Read(in, h); Read(in, c);
                Read(in, bytes);

                TextureLoadData data;
                data.Width = w; data.Height = h; data.Channels = c;
                data.Pixels = (unsigned char*)malloc(bytes); // freed by SceneBuilder during build
                if (bytes) in.read(reinterpret_cast<char*>(data.Pixels), bytes);
                m.EmbeddedTextures[(MaterialType)t] = data;
            }
        }

        // Meshes
        uint32_t meshCount = 0;
        Read(in, meshCount);
        std::vector<ParsedMesh> meshTable(meshCount);
        for (uint32_t i = 0; i < meshCount; i++)
        {
            ParsedMesh& pm = meshTable[i];
            Read(in, pm.Hash);
            pm.Name = ReadString(in);
            Read(in, pm.MinExtents);
            Read(in, pm.MaxExtents);

            uint32_t elemCount = 0;
            Read(in, elemCount);
            std::vector<BufferElement> elems;
            elems.reserve(elemCount);
            for (uint32_t k = 0; k < elemCount; k++)
            {
                std::string name = ReadString(in);
                int32_t t; uint8_t norm, en;
                Read(in, t); Read(in, norm); Read(in, en);
                elems.emplace_back((ShaderDataType)t, name, norm != 0, en != 0);
            }
            pm.Layout = BufferLayout(elems);

            uint64_t vcount = 0;
            Read(in, vcount);
            pm.Vertices.resize(vcount);
            if (vcount) in.read(reinterpret_cast<char*>(pm.Vertices.data()), vcount * sizeof(float));

            uint64_t icount = 0;
            Read(in, icount);
            pm.Indices.resize(icount);
            if (icount) in.read(reinterpret_cast<char*>(pm.Indices.data()), icount * sizeof(unsigned int));
        }

        scene.RootNode = ReadNode(in, meshTable);

        return scene;
    }

}
