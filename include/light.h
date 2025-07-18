#ifndef LIGHT
#define LIGHT

#include "preprocessor.h"

#include "shader.h"
#include "transform.h"
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

class Light : public Transform {
public:
    Light(Transform &transform) : Transform(transform) {};
    virtual void setLight(Shader &shader, const unsigned int idx, const unsigned int textureUnit) = 0;
    virtual void prepareLight(Shader &shader) = 0;
    virtual LightType getType() const = 0;
    virtual ~Light() = default;

protected:
    BaseLightData lightData;
};

class DirectionalLight : public Light {
public:
    DirectionalLight(Transform &transform) : Light(transform), projection(glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 25.0f)) {
        rotateTowards(glm::vec3(0.0f));
    };

    void setLight(Shader &shader, const unsigned int idx, const unsigned int textureUnit) override {
        // Only one directional light for now so ignore idx
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.direction"), 1, &reverseDir(glm::vec3(0.0f))[0]); 
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.ambient"), 1, &lightData.ambient[0]); 
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.diffuse"), 1, &lightData.diffuse[0]); 
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), "dirLight.specular"), 1, &lightData.specular[0]); 
        shader.setInt("shadowMap", textureUnit);
        prepareLight(shader);
    }

    void prepareLight(Shader &shader) override
    {
        glm::mat4 view = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("lightSpaceMatrix", projection * view);
    }

    LightType getType() const override { return LightType::Directional; }

private:
    glm::mat4 projection;
};

class SpotLight : public Light {
public:
    SpotLight(Transform &transform) : Light(transform), cutoff(glm::cos(glm::radians(12.5f))), outerCutoff(glm::cos(glm::radians(17.5f))), projection(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f)){};

    void setLight(Shader &shader, const unsigned int idx, const unsigned int textureUnit) override {
        std::string prefix = "spotLight[" + std::to_string(idx) + "]";

        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".position").c_str()), 1, &position[0]);
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".direction").c_str()), 1, &getDir(glm::vec3(0.0f))[0]);
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".ambient").c_str()), 1, &lightData.ambient[0]);
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".diffuse").c_str()), 1, &lightData.diffuse[0]);
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + ".specular").c_str()), 1, &lightData.specular[0]);

        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".cutOff").c_str()), cutoff);
        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".outerCutOff").c_str()), outerCutoff);

        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".constant").c_str()), attenuation.constant);
        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".linear").c_str()), attenuation.linear);
        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + ".quadratic").c_str()), attenuation.quadratic);
        std::string prefix2 = "shadowMapSpot[" + std::to_string(idx) + "]";
        std::string prefix3 = "spotLightSpaceMatrices[" + std::to_string(idx) + "]";
        shader.setInt(prefix2, textureUnit);
        shader.setMat4(prefix3, getMatrix());
    }

    void prepareLight(Shader &shader) override
    {
        glm::mat4 view = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("lightSpaceMatrix", projection * view);
    }

    LightType getType() const override { return LightType::Spot; }

    void setCutoffs(float inner, float outer) {
        cutoff = glm::cos(glm::radians(inner));
        outerCutoff = glm::cos(glm::radians(outer));
    }

private:
    float cutoff, outerCutoff;
    Attenuation attenuation;
    glm::mat4 projection;
};

class PointLight : public Light {
public:
    PointLight(Transform &transform) : Light(transform) {};

    void setLight(Shader &shader, const unsigned int idx, const unsigned int textureUnit) override {
        std::string prefix = "pointLights[" + std::to_string(idx) + "].";

        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + "position").c_str()), 1, &position[0]);
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + "ambient").c_str()), 1, &lightData.ambient[0]);
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + "diffuse").c_str()), 1, &lightData.diffuse[0]);
        glUniform3fv(glGetUniformLocation(shader.GetProgram(), (prefix + "specular").c_str()), 1, &lightData.specular[0]);

        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + "constant").c_str()), attenuation.constant);
        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + "linear").c_str()), attenuation.linear);
        glUniform1f(glGetUniformLocation(shader.GetProgram(), (prefix + "quadratic").c_str()), attenuation.quadratic);

        glUniform1f(glGetUniformLocation(shader.GetProgram(), "material.shininess"), 32.0f);
        // Not implemented yet, ignore this
    }

    void prepareLight(Shader &shader) override
    {
        shader.setMat4("lightSpaceMatrix", glm::mat4(10.0f));
    }

    LightType getType() const override { return LightType::Point; }

    Attenuation& attenuationData() { return attenuation; }

private:
    Attenuation attenuation;
};

#endif
