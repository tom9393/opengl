#pragma once

#include "IArchive.h"
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

namespace Engine {

    class YamlArchive : public IArchive {
    public:
        YamlArchive(YAML::Emitter& emitter) : m_Emitter(emitter) {}

        virtual void Write(const std::string &key, float value) override {
            m_Emitter << YAML::Key << value << YAML::Value << value;
        }
        virtual void Write(const std::string &key, int value) override {
            m_Emitter << YAML::Key << value << YAML::Value << value;
        }
        virtual void Write(const std::string &key, bool value) override {
            m_Emitter << YAML::Key << value << YAML::Value << value;
        }
        virtual void Write(const std::string &key, const std::string &value) override {
            m_Emitter << YAML::Key << value << YAML::Value << value;
        }
        virtual void Write(const std::string& key, const glm::vec3& value) override {
            m_Emitter << YAML::Key << key << YAML::Value << value;
        }

        virtual void Read(const std::string &key, float &value) override {
            
        }


    private:
        YAML::Emitter& m_Emitter;
    };

}