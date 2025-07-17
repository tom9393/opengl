#ifndef LIGHT
#define LIGHT

#include "shader.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class LightType { Directional, Point, Spot };

struct BaseLightData {
    glm::vec3 ambient  = glm::vec3(0.05f);
    glm::vec3 diffuse  = glm::vec3(1.0f);
    glm::vec3 specular = glm::vec3(1.0f);
};

struct Attenuation {
    float constant  = 1.0f;
    float linear    = 0.09f;
    float quadratic = 0.032f;
};

class Light {
public:
    virtual void setLight(Shader &shader, const unsigned int idx, const unsigned int textureUnit) = 0;
    virtual LightType getType() const = 0;
    virtual ~Light() = default;

    BaseLightData& data() { return lightData; }

protected:
    BaseLightData lightData;
};

class DirectionalLight : public Light {
public:
    DirectionalLight(glm::vec3 dir = glm::vec3(-2.0f, 4.0f, -1.0f))
        : direction(dir)
    {
        float near = 1.0f, far = 25.0f;
        projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near, far);
    }

    void setLight(Shader &shader, const unsigned int idx, const unsigned int textureUnit) override {
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.direction"), 1, &direction[0]); 
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.ambient"), 1, &data().ambient[0]); 
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.diffuse"), 1, &data().diffuse[0]); 
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.specular"), 1, &data().specular[0]); 
    }

    glm::mat4 getMatrix() const {
        glm::mat4 view = glm::lookAt(direction, glm::vec3(0.0f), glm::vec3(0, 1, 0));
        return projection * view;
    }

    LightType getType() const override { return LightType::Directional; }

private:
    glm::vec3 direction;
    glm::mat4 projection;
};

class PointLight : public Light {
public:
    PointLight(glm::vec3 pos = glm::vec3(0.0f)) : position(pos) {}

    void setLight(Shader &shader, const unsigned int idx, const unsigned int textureUnit) override {
        shader->setVec3(name + ".position", position);
        shader->setVec3(name + ".ambient", lightData.ambient);
        shader->setVec3(name + ".diffuse", lightData.diffuse);
        shader->setVec3(name + ".specular", lightData.specular);

        shader->setFloat(name + ".constant", attenuation.constant);
        shader->setFloat(name + ".linear", attenuation.linear);
        shader->setFloat(name + ".quadratic", attenuation.quadratic);
    }

    LightType getType() const override { return LightType::Point; }

    glm::vec3 getPosition() const { return position; }
    void setPosition(const glm::vec3& pos) { position = pos; }

    Attenuation& attenuationData() { return attenuation; }

private:
    glm::vec3 position;
    Attenuation attenuation;
};

class SpotLight : public Light {
public:
    SpotLight()
    {
        direction = glm::vec3(0.0f, 0.0f, -1.0f);
        position  = glm::vec3(0.0f);
        cutoff = glm::cos(glm::radians(12.5f));
        outerCutoff = glm::cos(glm::radians(17.5f));
    }

    void setLight(Shader &shader, const unsigned int idx, const unsigned int textureUnit) override {
        std::string prefix = "spotLight[" + std::to_string(idx) + "]";

        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".position").c_str()), 1, &getPosition()[0]);
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".direction").c_str()), 1, &getDirection()[0]);
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".ambient").c_str()), 1, &data().ambient[0]);
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".diffuse").c_str()), 1, &data().diffuse[0]);
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".specular").c_str()), 1, &data().specular[0]);

        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".cutOff").c_str()), cutoff);
        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".outerCutOff").c_str()), outerCutoff);

        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".constant").c_str()), attenuation.constant);
        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".linear").c_str()), attenuation.linear);
        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".quadratic").c_str()), attenuation.quadratic);
        std::string prefix2 = "shadowMapSpot[" + std::to_string(idx) + "]";
        std::string prefix3 = "spotLightSpaceMatrices[" + std::to_string(idx) + "]";
        map->BindForReading(GL_TEXTURE0 + textureUnit);
        shader.setInt(prefix2, textureUnit);
        shader.setMat4(prefix3, getMatrix());
        sl->setLight(&shader, prefix3);
    }

    LightType getType() const override { return LightType::Spot; }

    void setPosition(glm::vec3 pos) { position = pos; }
    void setDirection(glm::vec3 dir) { direction = glm::normalize(dir); }
    void setTarget(glm::vec3 t) { direction = glm::normalize(t - position); } // fixed version

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getDirection() const { return direction; }

    void setCutoffs(float inner, float outer) {
        cutoff = glm::cos(glm::radians(inner));
        outerCutoff = glm::cos(glm::radians(outer));
    }

private:
    glm::vec3 direction;
    glm::vec3 position;
    float cutoff, outerCutoff;
    Attenuation attenuation;
};


#endif
