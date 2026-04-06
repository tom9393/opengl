#include "SceneSerializer.h"
#include "../Asset/AssetManager.h"
#include "../Asset/AssimpLoader.h"
#include "Entity.h"
#include "Components.h"
#include <fstream>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/yaml.h>

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

YAML::Emitter& operator<<(YAML::Emitter &out, const glm::vec3 &v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter &out, const glm::vec4 &v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

namespace Engine
{

	static std::string RigidBodyToString(RigidBodyComponent::BodyType bodyType)
	{
		switch (bodyType) {
			case RigidBodyComponent::BodyType::Static: return "Static";
			case RigidBodyComponent::BodyType::Dynamic: return "Dynamic";
			case RigidBodyComponent::BodyType::Kinematic: return "Kinematic";
		}

		return "Unknown type";
	}

	static RigidBodyComponent::BodyType RigidBodyFromString(std::string &str)
	{
		if (str == "Dynamic") return RigidBodyComponent::BodyType::Dynamic;
		if (str == "Kinematic") return RigidBodyComponent::BodyType::Kinematic;
		return RigidBodyComponent::BodyType::Static;
	}

	template<typename T, typename Function>
	static void SerializeComponent(YAML::Emitter& out, Entity e, const std::string& name, Function func)
	{
	    if (e.HasComponent<T>())
	    {
			out << YAML::Key << name;
			out << YAML::BeginMap;

			auto& component = e.GetComponent<T>();
			func(out, component);

			out << YAML::EndMap;
	    }
	}

	void SerializeEntity(YAML::Emitter &out, Entity e)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << e.GetUUID();

		SerializeComponent<TagComponent>(out, e, "TagComponent", [](auto& out, auto& tc) {
			out << YAML::Key << "Tag" << YAML::Value << tc.tag;
		});

		SerializeComponent<TransformComponent>(out, e, "TransformComponent", [](auto& out, auto& tc) {
			out << YAML::Key << "Translation" << YAML::Value << tc.translation;
			out << YAML::Key << "Rotation"    << YAML::Value << tc.rotation;
			out << YAML::Key << "Scale"       << YAML::Value << tc.scale;
		});

		SerializeComponent<SceneCameraComponent>(out, e, "CameraComponent", [&](auto& out, auto& cc) {
			auto& cameraComponent = e.GetComponent<SceneCameraComponent>();
			auto& camera = cameraComponent.sceneCamera;

			out << YAML::Key << "Camera" << YAML::Value;

			out << YAML::BeginMap;

			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera->GetProjectionType();

			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera->GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera->GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera->GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera->GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera->GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera->GetOrthographicFarClip();

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.primary;

			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.fixedAspectRatio;
			out << YAML::EndMap;
		});

		SerializeComponent<MeshComponent>(out, e, "MeshComponent", [](auto& out, auto& mc) {
			out << YAML::Key << "Mesh" << YAML::Value << mc.MeshHandle;
		});

		SerializeComponent<MaterialComponent>(out, e, "MaterialComponent", [](auto& out, auto& mc) {
			out << YAML::Key << "Mesh" << YAML::Value << mc.MaterialHandle;
		});

		SerializeComponent<MaterialComponent>(out, e, "TextureComponent", [](auto& out, auto& tc) {
			out << YAML::Key << "Mesh" << YAML::Value << tc.TextureHandle;
		});

		SerializeComponent<RigidBodyComponent>(out, e, "RigidBodyComponent", [](auto& out, auto& rb) {
			out << YAML::Key << "BodyType" << YAML::Value << RigidBodyToString(rb.type);
			out << YAML::Key << "Mass"     << YAML::Value << rb.mass;
			out << YAML::Key << "Inertia"  << YAML::Value << rb.inertia;
		});

		SerializeComponent<LuaScriptComponent>(out, e, "LuaScriptComponent", [](auto& out, auto& lsc) {
			out << YAML::Key << "Path" << YAML::Value << lsc.FilePath;
		});

		if (e.HasComponent<RelationshipComponent>())
        {
            auto& rc = e.GetComponent<RelationshipComponent>();
            if (rc.Parent != entt::null)
            {
                Entity parent{ rc.Parent, e.GetScene() };
                out << YAML::Key << "RelationshipComponent";
                out << YAML::BeginMap;
                out << YAML::Key << "Parent" << YAML::Value << parent.GetUUID();
                out << YAML::EndMap;
            }
        }

		out << YAML::EndMap;
	};

	SceneSerializer::SceneSerializer(const Shared<Scene> &scene) : m_Scene(scene)
	{
	};

	void SceneSerializer::Serialize(const std::string &path)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Scene" << YAML::Value << "Unnamed scene";
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.view<entt::entity>().each([this, &out](entt::entity entity)
		{
			Entity e { entity, m_Scene.get() };
			if (!e) return;

			SerializeEntity(out, e);
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();
	};

	template <typename T>
	void DeserializeComponent(Entity &entity, Shared<Scene> &scene, YAML::Node node) {
	}

	template <>
	void DeserializeComponent<TransformComponent>(Entity &entity, Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			glm::vec3 translation = node["Translation"].as<glm::vec3>();
			glm::vec3 rotation = node["Rotation"].as<glm::vec3>();
			glm::vec3 size = node["Scale"].as<glm::vec3>();
			entity.AddComponent<TransformComponent>(translation, rotation, size);
		}
	}

	template <>
	void DeserializeComponent<SceneCameraComponent>(Entity &entity, Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& cc = entity.AddComponent<SceneCameraComponent>(MakeShared<SceneCamera>());

			auto cameraData = node["Camera"];
			cc.sceneCamera->SetProjectionType((SceneCamera::ProjectionType)cameraData["ProjectionType"].as<int>());

			cc.sceneCamera->SetOrthographicSize(cameraData["OrthographicSize"].as<float>());
			cc.sceneCamera->SetOrthographicNearClip(cameraData["OrthographicNear"].as<float>());
			cc.sceneCamera->SetOrthographicFarClip(cameraData["OrthographicFar"].as<float>());

			cc.sceneCamera->SetPerspective(
			    cameraData["PerspectiveFOV"].as<float>(),
			    cameraData["PerspectiveNear"].as<float>(),
			    cameraData["PerspectiveFar"].as<float>()
			);

			cc.primary = cameraData["Primary"].as<bool>();
			cc.fixedAspectRatio = cameraData["FixedAspectRatio"].as<bool>();
		}
	}

	template <>
	void DeserializeComponent<MeshComponent>(Entity &entity, Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& mc = entity.AddComponent<MeshComponent>();
			mc.MeshHandle = node["MeshHandle"].as<uint64_t>();
		}
	}

	template <>
	void DeserializeComponent<MaterialComponent>(Entity &entity, Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& mc = entity.AddComponent<MaterialComponent>();
			mc.MaterialHandle = node["MaterialHandle"].as<uint64_t>();
		}
	}

	template <>
	void DeserializeComponent<TextureComponent>(Entity &entity, Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& tc = entity.AddComponent<TextureComponent>();
			tc.TextureHandle = node["TextureHandle"].as<uint64_t>();
		}
	}

	template <>
	void DeserializeComponent<RigidBodyComponent>(Entity &entity, Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& rbc = entity.AddComponent<RigidBodyComponent>();
			std::string bodyTypeString = node["BodyType"].as<std::string>();
			rbc.type = RigidBodyFromString(bodyTypeString);
			rbc.mass = node["Mass"].as<float>();
			rbc.inertia = node["Inertia"].as<glm::vec3>();
		}
	}

	template <>
	void DeserializeComponent<LuaScriptComponent>(Entity &entity, Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& lsc = entity.AddComponent<LuaScriptComponent>();
			lsc.FilePath = node["Path"].as<std::string>();
		}
	}

	Entity DeserializeEntity(Shared<Scene> &scene, YAML::Node& node) {
		uint64_t uuid = node["Entity"].as<uint64_t>();
		std::string tag = node["TagComponent"]["Tag"].as<std::string>("");

		Entity entity = scene->CreateEntityWithUUID(uuid, tag);

		DeserializeComponent<TransformComponent>(entity, scene, node["TransformComponent"]);
		DeserializeComponent<SceneCameraComponent>(entity, scene, node["CameraComponent"]);
		DeserializeComponent<MeshComponent>(entity, scene, node["MeshComponent"]);
		DeserializeComponent<MaterialComponent>(entity, scene, node["MaterialComponent"]);
		DeserializeComponent<TextureComponent>(entity, scene, node["TextureComponent"]);
		DeserializeComponent<RigidBodyComponent>(entity, scene, node["RigidBodyComponent"]);
		DeserializeComponent<LuaScriptComponent>(entity, scene, node["LuaScriptComponent"]);

		return entity;
	}

	bool SceneSerializer::Deserialize(const std::string& path) {
		YAML::Node data = YAML::LoadFile(path);
        if (!data["Scene"]) return false;

        YAML::Node entities = data["Entities"];
        if (entities) 
        {
            // --- NEW: Two-Pass Deserialization ---
            std::unordered_map<uint64_t, Entity> loadedEntities;

            // Pass 1: Create all entities and base components
            for (YAML::Node entityNode : entities) 
            {
                Entity e = DeserializeEntity(m_Scene, entityNode);
                loadedEntities[e.GetUUID()] = e;
            }

            // Pass 2: Reconstruct Hierarchy
            for (YAML::Node entityNode : entities) 
            {
                auto relNode = entityNode["RelationshipComponent"];
                if (relNode && relNode["Parent"]) 
                {
                    uint64_t childUUID = entityNode["Entity"].as<uint64_t>();
                    uint64_t parentUUID = relNode["Parent"].as<uint64_t>();

                    Entity child = loadedEntities[childUUID];
                    Entity parent = loadedEntities[parentUUID];

                    if (child && parent) 
                    {
                        // SetParent should handle updating both the child's Parent handle 
                        // and adding the child to the parent's Children vector
                        child.SetParent(parent); 
                    }
                }
            }
        }
        return true;
	}
};
