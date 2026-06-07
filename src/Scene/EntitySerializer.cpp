#include "EntitySerializer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"
#include "Asset/YamlArchive.h"

#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/yaml.h>

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

	void EntitySerializer::SerializeEntity(YAML::Emitter &out, Entity e, bool includeParent)
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
			out << YAML::Key << "MeshHandle" << YAML::Value << mc.MeshHandle;
		});

		SerializeComponent<MaterialComponent>(out, e, "MaterialComponent", [](auto& out, auto& mc) {
			out << YAML::Key << "MaterialHandle" << YAML::Value << mc.MaterialHandle;
		});

		SerializeComponent<TextureComponent>(out, e, "TextureComponent", [](auto& out, auto& tc) {
			out << YAML::Key << "TextureHandle" << YAML::Value << tc.TextureHandle;
		});

		SerializeComponent<RigidBodyComponent>(out, e, "RigidBodyComponent", [](auto& out, auto& rb) {
			out << YAML::Key << "BodyType"  << YAML::Value << RigidBodyToString(rb.type);
			out << YAML::Key << "Mass"      << YAML::Value << rb.mass;
			out << YAML::Key << "Inertia"   << YAML::Value << rb.inertia;
			out << YAML::Key << "ShapePath" << YAML::Value << rb.ShapePath.string();
		});

		SerializeComponent<LuaScriptComponent>(out, e, "LuaScriptComponent", [](auto& out, auto& lsc) {
			out << YAML::Key << "ScriptHandle" << YAML::Value << lsc.ScriptHandle;
		});

		SerializeComponent<LightComponent>(out, e, "LightComponent", [](auto& out, auto& lc) {
			out << YAML::Key << "Type" << YAML::Value << (int)lc.Type;
            out << YAML::Key << "Color" << YAML::Value << lc.Color;
            out << YAML::Key << "Intensity" << YAML::Value << lc.Intensity;
            out << YAML::Key << "CastShadows" << YAML::Value << lc.CastShadows;
            out << YAML::Key << "InnerCutOff" << YAML::Value << lc.InnerCutOff;
            out << YAML::Key << "OuterCutOff" << YAML::Value << lc.OuterCutOff;
		});

		if (includeParent && e.HasComponent<ParentComponent>())
        {
            Entity parent{ e.GetComponent<ParentComponent>().Parent, e.GetScene() };
            out << YAML::Key << "RelationshipComponent";
            out << YAML::BeginMap;
            out << YAML::Key << "Parent" << YAML::Value << parent.GetUUID();
            out << YAML::EndMap;
        }

		out << YAML::EndMap;
	};

	template <typename T>
	void DeserializeComponent(Entity &entity, const Shared<Scene> &scene, YAML::Node node) {
	}

	template <>
	void DeserializeComponent<TransformComponent>(Entity &entity, const Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			glm::vec3 translation = node["Translation"].as<glm::vec3>();
			glm::vec3 rotation = node["Rotation"].as<glm::vec3>();
			glm::vec3 size = node["Scale"].as<glm::vec3>();
			entity.AddComponent<TransformComponent>(translation, rotation, size);
		}
	}

	template <>
	void DeserializeComponent<SceneCameraComponent>(Entity &entity, const Shared<Scene> &scene, YAML::Node node) {
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
	void DeserializeComponent<MeshComponent>(Entity &entity, const Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& mc = entity.AddComponent<MeshComponent>();
			mc.MeshHandle = node["MeshHandle"].as<uint64_t>();
		}
	}

	template <>
	void DeserializeComponent<MaterialComponent>(Entity &entity, const Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& mc = entity.AddComponent<MaterialComponent>();
			mc.MaterialHandle = node["MaterialHandle"].as<uint64_t>();
		}
	}

	template <>
	void DeserializeComponent<TextureComponent>(Entity &entity, const Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& tc = entity.AddComponent<TextureComponent>();
			tc.TextureHandle = node["TextureHandle"].as<uint64_t>();
		}
	}

	template <>
	void DeserializeComponent<RigidBodyComponent>(Entity &entity, const Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& rbc = entity.AddComponent<RigidBodyComponent>();
			std::string bodyTypeString = node["BodyType"].as<std::string>();
			rbc.type = RigidBodyFromString(bodyTypeString);
			rbc.mass = node["Mass"].as<float>();
			rbc.inertia = node["Inertia"].as<glm::vec3>();
			if (node["ShapePath"])
				rbc.ShapePath = node["ShapePath"].as<std::string>();
		}
	}

	template <>
	void DeserializeComponent<LuaScriptComponent>(Entity &entity, const Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& lsc = entity.AddComponent<LuaScriptComponent>();
			lsc.ScriptHandle = node["ScriptHandle"].as<uint64_t>();
		}
	}

	template <>
	void DeserializeComponent<LightComponent>(Entity &entity, const Shared<Scene> &scene, YAML::Node node) {
		if (node)
		{
			auto& light = entity.AddComponent<LightComponent>();
			light.Type = (LightComponent::LightType)node["Type"].as<int>();
			light.Color = node["Color"].as<glm::vec3>();
			light.Intensity = node["Intensity"].as<float>();
			light.CastShadows = node["CastShadows"].as<bool>();

			if (node["InnerCutOff"])
				light.InnerCutOff = node["InnerCutOff"].as<float>();
			if (node["OuterCutOff"])
				light.OuterCutOff = node["OuterCutOff"].as<float>();
		}
	}

	Entity EntitySerializer::DeserializeEntity(const Shared<Scene> &scene, YAML::Node& node) {
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
		DeserializeComponent<LightComponent>(entity, scene, node["LightComponent"]);

		return entity;
	}

	void EntitySerializer::DeserializeEntities(const Shared<Scene>& scene, YAML::Node entities)
	{
		std::unordered_map<uint64_t, Entity> loadedEntities;

		// Gather all entities
		for (YAML::Node entityNode : entities)
		{
			Entity e = DeserializeEntity(scene, entityNode);
			loadedEntities[e.GetUUID()] = e;
		}

		// Then build their relationships
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
					child.SetParent(parent);
			}
		}
	}

};
