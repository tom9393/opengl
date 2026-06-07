#include "SceneSerializer.h"
#include "EntitySerializer.h"
#include "Asset/YamlArchive.h"
#include "Entity.h"

#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/yaml.h>

namespace Engine
{

	SceneSerializer::SceneSerializer(const Shared<Scene> &scene) : m_Scene(scene)
	{
	};

	void SceneSerializer::Serialize(const std::string &path)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Scene" << YAML::Value << "Unnamed scene";

		out << YAML::Key << "Environment" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "Path" << YAML::Value << m_Scene->GetEnvironmentPath();
		out << YAML::Key << "Intensity" << YAML::Value << m_Scene->GetEnvironmentIntensity();
		out << YAML::EndMap;

		{
			auto& post = m_Scene->GetPostSettings();
			out << YAML::Key << "PostProcess" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "BloomEnabled" << YAML::Value << post.BloomEnabled;
			out << YAML::Key << "BloomIntensity" << YAML::Value << post.BloomIntensity;
			out << YAML::Key << "BloomThreshold" << YAML::Value << post.BloomThreshold;
			out << YAML::Key << "Exposure" << YAML::Value << post.Exposure;
			out << YAML::Key << "VignetteStrength" << YAML::Value << post.VignetteStrength;
			out << YAML::Key << "ChromaticAberration" << YAML::Value << post.ChromaticAberration;
			out << YAML::EndMap;
		}

		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.view<entt::entity>().each([this, &out](entt::entity entity)
		{
			Entity e { entity, m_Scene.get() };
			if (!e) return;

			EntitySerializer::SerializeEntity(out, e);
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();
	};

	bool SceneSerializer::Deserialize(const std::string& path) {
		YAML::Node data = YAML::LoadFile(path);
        if (!data["Scene"]) return false;

        if (YAML::Node envNode = data["Environment"])
        {
            m_Scene->SetEnvironmentIntensity(envNode["Intensity"].as<float>(1.0f));
            std::string envPath = envNode["Path"].as<std::string>("");
            if (!envPath.empty()) m_Scene->SetEnvironmentPath(envPath);
        }

        if (YAML::Node postNode = data["PostProcess"])
        {
            auto& post = m_Scene->GetPostSettings();
            post.BloomEnabled        = postNode["BloomEnabled"].as<bool>(post.BloomEnabled);
            post.BloomIntensity      = postNode["BloomIntensity"].as<float>(post.BloomIntensity);
            post.BloomThreshold      = postNode["BloomThreshold"].as<float>(post.BloomThreshold);
            post.Exposure            = postNode["Exposure"].as<float>(post.Exposure);
            post.VignetteStrength    = postNode["VignetteStrength"].as<float>(post.VignetteStrength);
            post.ChromaticAberration = postNode["ChromaticAberration"].as<float>(post.ChromaticAberration);
        }

        YAML::Node entities = data["Entities"];
        if (entities)
            EntitySerializer::DeserializeEntities(m_Scene, entities);

        return true;
	};

};
