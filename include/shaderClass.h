#ifndef OLDSHADER_H
#define OLDSHADER_H

#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
  
class OldShader
{
public:
    // the program ID
    unsigned int ID;
  
    // constructor reads and builds the shader
    OldShader(const char* vertexPath, const char* fragmentPath);
    // use/activate the shader
    void use();
    void remove();
    // utility uniform functions
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &transform) const;
    void setVec3(const std::string &name, const glm::vec3 &transform) const;
    void setVec4(const std::string &name, const glm::vec4 &transform) const;
    void setMat2(const std::string &name, const glm::mat2 &transform) const;
    void setMat3(const std::string &name, const glm::mat3 &transform) const;
    void setMat4(const std::string &name, const glm::mat4 &transform) const;
};
  
#endif
