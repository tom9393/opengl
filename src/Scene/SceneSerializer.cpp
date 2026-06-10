#include "SceneSerializer.h"
#include "Components.h"
#include "Entity.h"

#include <cstring>
#include <fstream>

namespace Engine
{
    // Binary editor-scene format: magic "PSCN", then scene settings and every entity's
    // components with asset-handle references. Shares the .pmscene extension with baked
    // models ("PMSC", PmSceneSerializer); the magic tells them apart.
    // Version 2: ParticleEmitterComponent block (between light and parent link).
    static constexpr uint32_t SCENE_VERSION = 2;

    // --- binary helpers -----------------------------------------------------
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

    SceneSerializer::SceneSerializer(const Shared<Scene> &scene) : m_Scene(scene)
    {
    };

    bool SceneSerializer::IsSceneFile(const std::string &path)
    {
        std::ifstream in(path, std::ios::binary);
        char magic[4] = {};
        in.read(magic, 4);
        return in && std::strncmp(magic, "PSCN", 4) == 0;
    }

    static void SerializeEntity(std::ostream& out, Entity e)
    {
        Write(out, (uint64_t)e.GetUUID());
        WriteString(out, e.HasComponent<TagComponent>() ? e.GetComponent<TagComponent>().tag : "");

        uint8_t has = e.HasComponent<TransformComponent>() ? 1 : 0;
        Write(out, has);
        if (has)
        {
            auto& tc = e.GetComponent<TransformComponent>();
            Write(out, tc.translation);
            Write(out, tc.rotation);
            Write(out, tc.scale);
        }

        has = e.HasComponent<SceneCameraComponent>() ? 1 : 0;
        Write(out, has);
        if (has)
        {
            auto& cc = e.GetComponent<SceneCameraComponent>();
            auto& camera = cc.sceneCamera;
            int32_t projection = (int32_t)camera->GetProjectionType();
            uint8_t primary = cc.primary ? 1 : 0;
            uint8_t fixedAspect = cc.fixedAspectRatio ? 1 : 0;
            Write(out, projection);
            Write(out, camera->GetPerspectiveVerticalFOV());
            Write(out, camera->GetPerspectiveNearClip());
            Write(out, camera->GetPerspectiveFarClip());
            Write(out, camera->GetOrthographicSize());
            Write(out, camera->GetOrthographicNearClip());
            Write(out, camera->GetOrthographicFarClip());
            Write(out, primary);
            Write(out, fixedAspect);
        }

        has = e.HasComponent<MeshComponent>() ? 1 : 0;
        Write(out, has);
        if (has) Write(out, (uint64_t)e.GetComponent<MeshComponent>().MeshHandle);

        has = e.HasComponent<MaterialComponent>() ? 1 : 0;
        Write(out, has);
        if (has) Write(out, (uint64_t)e.GetComponent<MaterialComponent>().MaterialHandle);

        has = e.HasComponent<TextureComponent>() ? 1 : 0;
        Write(out, has);
        if (has) Write(out, (uint64_t)e.GetComponent<TextureComponent>().TextureHandle);

        has = e.HasComponent<RigidBodyComponent>() ? 1 : 0;
        Write(out, has);
        if (has)
        {
            auto& rb = e.GetComponent<RigidBodyComponent>();
            int32_t bodyType = (int32_t)rb.type;
            Write(out, bodyType);
            Write(out, rb.mass);
            Write(out, rb.inertia);
        }

        has = e.HasComponent<LuaScriptComponent>() ? 1 : 0;
        Write(out, has);
        if (has) Write(out, (uint64_t)e.GetComponent<LuaScriptComponent>().ScriptHandle);

        has = e.HasComponent<LightComponent>() ? 1 : 0;
        Write(out, has);
        if (has)
        {
            auto& lc = e.GetComponent<LightComponent>();
            int32_t type = (int32_t)lc.Type;
            uint8_t castShadows = lc.CastShadows ? 1 : 0;
            Write(out, type);
            Write(out, lc.Color);
            Write(out, lc.Intensity);
            Write(out, castShadows);
            Write(out, lc.InnerCutOff);
            Write(out, lc.OuterCutOff);
        }

        has = e.HasComponent<ParticleEmitterComponent>() ? 1 : 0;
        Write(out, has);
        if (has)
        {
            auto& pe = e.GetComponent<ParticleEmitterComponent>();
            uint8_t looping = pe.Looping ? 1 : 0;
            uint8_t additive = pe.Additive ? 1 : 0;
            Write(out, looping);
            Write(out, pe.Rate);
            Write(out, pe.BurstCount);
            Write(out, pe.MaxParticles);
            Write(out, pe.Lifetime);
            Write(out, pe.Direction);
            Write(out, pe.Spread);
            Write(out, pe.Speed);
            Write(out, pe.Gravity);
            Write(out, pe.StartSize);
            Write(out, pe.EndSize);
            Write(out, pe.StartColor);
            Write(out, pe.EndColor);
            Write(out, additive);
            Write(out, (uint64_t)pe.TextureHandle);
        }

        has = e.HasComponent<ParentComponent>() ? 1 : 0;
        Write(out, has);
        if (has)
        {
            Entity parent{ e.GetComponent<ParentComponent>().Parent, e.GetScene() };
            Write(out, (uint64_t)parent.GetUUID());
        }
    }

    void SceneSerializer::Serialize(const std::string &path)
    {
        std::ofstream out(path, std::ios::binary);
        if (!out) { std::cerr << "Failed to open scene for writing: " << path << std::endl; return; }

        out.write("PSCN", 4);
        uint32_t version = SCENE_VERSION;
        Write(out, version);

        WriteString(out, m_Scene->GetEnvironmentPath());
        Write(out, m_Scene->GetEnvironmentIntensity());

        auto& post = m_Scene->GetPostSettings();
        uint8_t bloomEnabled = post.BloomEnabled ? 1 : 0;
        Write(out, bloomEnabled);
        Write(out, post.BloomIntensity);
        Write(out, post.BloomThreshold);
        Write(out, post.Exposure);
        Write(out, post.VignetteStrength);
        Write(out, post.ChromaticAberration);

        std::vector<Entity> entities;
        m_Scene->GetRegistry().view<entt::entity>().each([this, &entities](entt::entity entity)
        {
            Entity e { entity, m_Scene.get() };
            if (e) entities.push_back(e);
        });

        uint32_t count = (uint32_t)entities.size();
        Write(out, count);
        for (Entity e : entities)
            SerializeEntity(out, e);
    };

    static Entity DeserializeEntity(std::istream& in, const Shared<Scene>& scene,
                                    std::vector<std::pair<uint64_t, uint64_t>>& parentLinks,
                                    uint32_t version)
    {
        uint64_t uuid = 0;
        Read(in, uuid);
        std::string tag = ReadString(in);

        Entity entity = scene->CreateEntityWithUUID(uuid, tag);

        uint8_t has = 0;

        Read(in, has);
        if (has)
        {
            glm::vec3 translation, rotation, scale;
            Read(in, translation);
            Read(in, rotation);
            Read(in, scale);
            entity.AddComponent<TransformComponent>(translation, rotation, scale);
        }

        Read(in, has);
        if (has)
        {
            auto& cc = entity.AddComponent<SceneCameraComponent>(MakeShared<SceneCamera>());

            int32_t projection = 0;
            float perspectiveFOV, perspectiveNear, perspectiveFar;
            float orthographicSize, orthographicNear, orthographicFar;
            uint8_t primary = 0, fixedAspect = 0;
            Read(in, projection);
            Read(in, perspectiveFOV);
            Read(in, perspectiveNear);
            Read(in, perspectiveFar);
            Read(in, orthographicSize);
            Read(in, orthographicNear);
            Read(in, orthographicFar);
            Read(in, primary);
            Read(in, fixedAspect);

            cc.sceneCamera->SetProjectionType((SceneCamera::ProjectionType)projection);
            cc.sceneCamera->SetOrthographicSize(orthographicSize);
            cc.sceneCamera->SetOrthographicNearClip(orthographicNear);
            cc.sceneCamera->SetOrthographicFarClip(orthographicFar);
            cc.sceneCamera->SetPerspective(perspectiveFOV, perspectiveNear, perspectiveFar);
            cc.primary = primary != 0;
            cc.fixedAspectRatio = fixedAspect != 0;
        }

        Read(in, has);
        if (has)
        {
            uint64_t handle = 0;
            Read(in, handle);
            entity.AddComponent<MeshComponent>().MeshHandle = handle;
        }

        Read(in, has);
        if (has)
        {
            uint64_t handle = 0;
            Read(in, handle);
            entity.AddComponent<MaterialComponent>().MaterialHandle = handle;
        }

        Read(in, has);
        if (has)
        {
            uint64_t handle = 0;
            Read(in, handle);
            entity.AddComponent<TextureComponent>().TextureHandle = handle;
        }

        Read(in, has);
        if (has)
        {
            auto& rb = entity.AddComponent<RigidBodyComponent>();
            int32_t bodyType = 0;
            Read(in, bodyType);
            rb.type = (RigidBodyComponent::BodyType)bodyType;
            Read(in, rb.mass);
            Read(in, rb.inertia);
        }

        Read(in, has);
        if (has)
        {
            uint64_t handle = 0;
            Read(in, handle);
            entity.AddComponent<LuaScriptComponent>().ScriptHandle = handle;
        }

        Read(in, has);
        if (has)
        {
            auto& light = entity.AddComponent<LightComponent>();
            int32_t type = 0;
            uint8_t castShadows = 0;
            Read(in, type);
            light.Type = (LightComponent::LightType)type;
            Read(in, light.Color);
            Read(in, light.Intensity);
            Read(in, castShadows);
            light.CastShadows = castShadows != 0;
            Read(in, light.InnerCutOff);
            Read(in, light.OuterCutOff);
        }

        if (version >= 2)
        {
            Read(in, has);
            if (has)
            {
                auto& pe = entity.AddComponent<ParticleEmitterComponent>();
                uint8_t looping = 0, additive = 0;
                uint64_t textureHandle = 0;
                Read(in, looping);
                Read(in, pe.Rate);
                Read(in, pe.BurstCount);
                Read(in, pe.MaxParticles);
                Read(in, pe.Lifetime);
                Read(in, pe.Direction);
                Read(in, pe.Spread);
                Read(in, pe.Speed);
                Read(in, pe.Gravity);
                Read(in, pe.StartSize);
                Read(in, pe.EndSize);
                Read(in, pe.StartColor);
                Read(in, pe.EndColor);
                Read(in, additive);
                Read(in, textureHandle);
                pe.Looping = looping != 0;
                pe.Additive = additive != 0;
                pe.TextureHandle = textureHandle;
            }
        }

        Read(in, has);
        if (has)
        {
            uint64_t parentUUID = 0;
            Read(in, parentUUID);
            parentLinks.emplace_back(uuid, parentUUID);
        }

        return entity;
    }

    bool SceneSerializer::Deserialize(const std::string& path)
    {
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;

        char magic[4] = {};
        in.read(magic, 4);
        if (std::strncmp(magic, "PSCN", 4) != 0) { std::cerr << "Not a scene file: " << path << std::endl; return false; }

        uint32_t version = 0;
        Read(in, version);

        std::string environmentPath = ReadString(in);
        float environmentIntensity = 1.0f;
        Read(in, environmentIntensity);
        m_Scene->SetEnvironmentIntensity(environmentIntensity);
        if (!environmentPath.empty()) m_Scene->SetEnvironmentPath(environmentPath);

        auto& post = m_Scene->GetPostSettings();
        uint8_t bloomEnabled = 0;
        Read(in, bloomEnabled);
        post.BloomEnabled = bloomEnabled != 0;
        Read(in, post.BloomIntensity);
        Read(in, post.BloomThreshold);
        Read(in, post.Exposure);
        Read(in, post.VignetteStrength);
        Read(in, post.ChromaticAberration);

        uint32_t count = 0;
        Read(in, count);

        std::unordered_map<uint64_t, Entity> loadedEntities;
        std::vector<std::pair<uint64_t, uint64_t>> parentLinks; // child UUID, parent UUID
        for (uint32_t i = 0; i < count; i++)
        {
            Entity e = DeserializeEntity(in, m_Scene, parentLinks, version);
            loadedEntities[e.GetUUID()] = e;
        }

        for (auto& [childUUID, parentUUID] : parentLinks)
        {
            Entity child = loadedEntities[childUUID];
            Entity parent = loadedEntities[parentUUID];

            if (child && parent)
                child.SetParent(parent);
        }

        return true;
    };

};
