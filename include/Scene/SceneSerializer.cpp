#include "SceneSerializer.h"
#include "Entity.h"
#include "Components.h"
#include <fstream>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/yaml.h>

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

static void SerializeEntity(YAML::Emitter &out, Entity e)
{
	out << YAML::BeginMap;
	out << YAML::Key << "Entity";
	out << YAML::Value << "12345"; // Key goes here
	
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
	return false;
};
