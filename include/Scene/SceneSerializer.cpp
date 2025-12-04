#include "SceneSerializer.h"
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

	static void SerializeEntity(YAML::Emitter &out, Entity e)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity";
		out << YAML::Value << e.GetUUID();
		
		if (e.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			auto& tag = e.GetComponent<TagComponent>();
			out << YAML::Key << "Tag";
			out << YAML::Value << tag.tag;

			out << YAML::EndMap;
		}

		if (e.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			auto& tc = e.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation";
			out << YAML::Value << tc.translation;
			out << YAML::Key << "Rotation";
			out << YAML::Value << tc.rotation;
			out << YAML::Key << "Scale";
			out << YAML::Value << tc.scale;

			out << YAML::EndMap;
		}

		if (e.HasComponent<SceneCameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;

			auto& cameraComponent = e.GetComponent<SceneCameraComponent>();
			auto& camera = cameraComponent.sceneCamera;

			out << YAML::Key << "Camera";
			out << YAML::Value;

			out << YAML::BeginMap;

			out << YAML::Key << "ProjectionType";
			out << YAML::Value << (int)camera->GetProjectionType();
			out << YAML::Key << "PerspectiveFOV";
			out << YAML::Value << camera->GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear";
			out << YAML::Value << camera->GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar";
			out << YAML::Value << camera->GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize";
			out << YAML::Value << camera->GetOrthographicSize();
			out << YAML::Key << "OrthographicNear";
			out << YAML::Value << camera->GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar";
			out << YAML::Value << camera->GetOrthographicFarClip();

			out << YAML::Key << "Primary";
			out << YAML::Value << cameraComponent.primary;

			out << YAML::Key << "FixedAspectRatio";
			out << YAML::Value << cameraComponent.fixedAspectRatio;
		}

		if (e.HasComponent<ModelComponent>())
		{
			out << YAML::Key << "ModelComponent";
			out << YAML::BeginMap;

			auto& model = e.GetComponent<ModelComponent>();
			out << YAML::Key << "Model";
			out << YAML::Value << model.model;

			out << YAML::EndMap;
		}

		if (e.HasComponent<RigidBodyComponent>())
		{
			out << YAML::Key << "RigidBodyComponent";
			out << YAML::BeginMap;

			auto& rigidBody = e.GetComponent<RigidBodyComponent>();
			out << YAML::Key << "BodyType";
			out << YAML::Value << RigidBodyToString(rigidBody.type);

			out << YAML::EndMap;
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
		out << YAML::Key << "Scene";
		out << YAML::Value << "Unnamed scene";
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

	bool SceneSerializer::Deserialize(const std::string &path)
	{
		std::ifstream stream(path);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());

		if (!data["Scene"]) return false;

		std::string sceneName = data["Scene"].as<std::string>();

		auto entities = data["Entities"];

		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();
				
				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent) name = tagComponent["Tag"].as<std::string>();

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					glm::vec3 translation = transformComponent["Translation"].as<glm::vec3>();
					glm::vec3 rotation = transformComponent["Rotation"].as<glm::vec3>();
					glm::vec3 size = transformComponent["Scale"].as<glm::vec3>();
					auto& tc = deserializedEntity.AddComponent<TransformComponent>(translation, rotation, size);
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<SceneCameraComponent>(MakeShared<SceneCamera>());

					auto cameraData = cameraComponent["Camera"];
					cc.sceneCamera->SetProjectionType((SceneCamera::ProjectionType)cameraData["ProjectionType"].as<int>());

					cc.sceneCamera->SetPerspectiveVerticalFOV(cameraData["PerspectiveFOV"].as<float>());
					cc.sceneCamera->SetPerspectiveNearClip(cameraData["PerspectiveNear"].as<float>());
					cc.sceneCamera->SetPerspectiveFarClip(cameraData["PerspectiveFar"].as<float>());

					cc.sceneCamera->SetOrthographicSize(cameraData["OrthographicSize"].as<float>());
					cc.sceneCamera->SetOrthographicNearClip(cameraData["OrthographicNear"].as<float>());
					cc.sceneCamera->SetOrthographicFarClip(cameraData["OrthographicFar"].as<float>());

					cc.primary = cameraComponent["Primary"].as<bool>();
					cc.fixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto modelComponent = entity["ModelComponent"];
				if (modelComponent)
				{
					auto& mc = deserializedEntity.AddComponent<ModelComponent>();
					std::string path = modelComponent["Model"].as<std::string>();
					mc.model = path;
				}

				auto rigidBodyComponent = entity["RigidBodyComponent"];
				if (rigidBodyComponent)
				{
					auto& rbc = deserializedEntity.AddComponent<RigidBodyComponent>();
					std::string bodyTypeString = rigidBodyComponent["BodyType"].as<std::string>();
					rbc.type = RigidBodyFromString(bodyTypeString);
				}
			}
		}
		return true;
	};

};
