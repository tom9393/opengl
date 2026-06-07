#include "PrefabSerializer.h"
#include "EntitySerializer.h"
#include "Components.h"
#include "Entity.h"

#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/yaml.h>

namespace Engine
{

	PrefabSerializer::PrefabSerializer(const Shared<Scene> &scene) : m_Scene(scene)
	{
	};

	void PrefabSerializer::Serialize(Entity root, const std::string& path)
	{
		std::vector<Entity> subtree;

		std::function<void(Entity)> gather = [&](Entity e) {
			subtree.push_back(e);
			for (entt::entity child : e.GetChildren())
				gather(Entity{ child, e.GetScene() });
		};
		gather(root);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Prefab" << YAML::Value << root.GetComponent<TagComponent>().tag;
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		for (Entity e : subtree)
			EntitySerializer::SerializeEntity(out, e, e != root);
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();
	}

	bool PrefabSerializer::Deserialize(const std::string& path)
	{
		YAML::Node data = YAML::LoadFile(path);
		YAML::Node entities = data["Entities"];
		if (!entities) return false;

		EntitySerializer::DeserializeEntities(m_Scene, entities);
		return true;
	}

};
