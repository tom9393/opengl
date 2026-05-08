#include "pch.h"
#include "OpenGLShader.h"

namespace Engine
{

    OpenGLShader::OpenGLShader(const std::string &name)
    {
        id = glCreateProgram();

        if (id == 0) {
            fprintf(stderr, "Error creating shader program\n");
        }

        m_Name = name;
    }

    OpenGLShader::~OpenGLShader()
    {
        // Delete the intermediate shader objects that have been added to the program
        // The list will only contain something if shaders were compiled but the object itself
        // was destroyed prior to linking.
        for (std::list<GLuint>::iterator it = shaderObjList.begin() ; it != shaderObjList.end() ; it++)
        {
            glDeleteShader(*it);
        }

        if (id != 0)
        {
            glDeleteProgram(id);
            id = 0;
        }
    }

    GLenum OpenGLShader::GetShaderFromExtension(const std::string &file)
    {
        std::string extension = file.substr(file.find_last_of(".") + 1);

        if (extension == "vert") {
            return GL_VERTEX_SHADER;
        } 
        else if (extension == "geom") {
            return GL_GEOMETRY_SHADER;
        } 
        else if (extension == "frag") {
            return GL_FRAGMENT_SHADER;
        }

        // Fallback or error logging
        return GL_FALSE; 
    }


    void OpenGLShader::AddShaders(const std::vector<std::string> &shaders)
    {
        for (auto &shader : shaders)
        {
            AddShader(shader);
        }
        Finalize();
    };

    void OpenGLShader::AddShaders(const std::initializer_list<std::string> &shaders)
    {
        for (auto &shader : shaders)
        {
            AddShader(shader);
        }
        Finalize();
    };

    // Use this method to add shaders to the program. When finished - call finalize()
    bool OpenGLShader::AddShader(const std::string &file)
    {
        std::string s;
        std::ifstream f;

        f.exceptions (std::ifstream::failbit | std::ifstream::badbit);

        try 
        {
            // open files
            f.open(file.c_str());
            std::stringstream stream;
            // read file's buffer contents into streams
            stream << f.rdbuf();
            // close file handlers
            f.close();
            // convert stream into string
            s = stream.str();
        }
        catch(std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
            return false;
        }

        unsigned int ShaderType = GetShaderFromExtension(file);

        GLuint ShaderObj = glCreateShader(ShaderType);

        if (ShaderObj == 0) {
            std::cout << "Error creating shader" << std::endl;
            return false;
        }

        // Save the shader object - will be deleted in the destructor
        shaderObjList.push_back(ShaderObj);

        const GLchar* p[1];
        p[0] = s.c_str();
        GLint Lengths[1] = { (GLint)s.size() };

        glShaderSource(ShaderObj, 1, p, Lengths);

        glCompileShader(ShaderObj);

        GLint success;
        glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

        if (!success) {
            GLchar InfoLog[1024];
            glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
            fprintf(stderr, "Error compiling '%s': '%s'\n", file.c_str(), InfoLog);
            return false;
        }

        glAttachShader(id, ShaderObj);

        return true;
    }


    // After all the shaders have been added to the program call this function
    // to link and validate the program.
    bool OpenGLShader::Finalize()
    {
        GLint Success = 0;
        GLchar ErrorLog[1024] = { 0 };

        glLinkProgram(id);

        glGetProgramiv(id, GL_LINK_STATUS, &Success);

        if (Success == 0) {
            glGetProgramInfoLog(id, sizeof(ErrorLog), NULL, ErrorLog);
            fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
            return false;
        }

        // Delete the intermediate shader objects that have been added to the program
        for (std::list<GLuint>::iterator it = shaderObjList.begin() ; it != shaderObjList.end() ; it++) {
            glDeleteShader(*it);
        }

        shaderObjList.clear();

        ExtractActiveAttributesAndUniforms();

        return true;
    }

    void OpenGLShader::ExtractActiveAttributesAndUniforms()
    {
        m_ActiveAttributes.clear();
        m_ActiveUniforms.clear();
        m_ActiveUBOs.clear();
        m_ActiveSSBOs.clear();

        GLint size; 
        GLenum type;
        const GLsizei bufSize = 256; 
        GLchar name[bufSize];
        GLsizei length; 

        // --- Extract Attributes ---
        int attribCount = 0;
        glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &attribCount);

        for (int i = 0; i < attribCount; i++) {
            glGetActiveAttrib(id, (GLuint)i, bufSize, &length, &size, &type, name);
            int location = glGetAttribLocation(id, name);
            m_ActiveAttributes.push_back({ std::string(name), (uint32_t)type, size, location });
        }

        // Sort attributes by location
        std::sort(m_ActiveAttributes.begin(), m_ActiveAttributes.end(), [](const AttributeInfo& a, const AttributeInfo& b) {
            return a.Location < b.Location;
        });

        // --- Extract Uniforms ---
        int uniformCount = 0;
        glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &uniformCount);

        for (int i = 0; i < uniformCount; i++) {
            glGetActiveUniform(id, (GLuint)i, bufSize, &length, &size, &type, name);
            m_ActiveUniforms.push_back({ std::string(name), (uint32_t)type, size });

            int location = glGetUniformLocation(id, name);
            m_UniformLocationCache[std::string(name)] = location;
        }

        // The properties we want to extract from each block
        const GLenum blockProperties[2] = { GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE };
        GLint values[2];
        char nameBuffer[256];

        // --- Extract Uniform Blocks (UBOs) ---
        int uboCount = 0;
        glGetProgramInterfaceiv(id, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &uboCount);

        for (int i = 0; i < uboCount; i++) {
            // Fetch binding point and size
            glGetProgramResourceiv(id, GL_UNIFORM_BLOCK, i, 2, blockProperties, 2, nullptr, values);
            // Fetch the block name
            glGetProgramResourceName(id, GL_UNIFORM_BLOCK, i, sizeof(nameBuffer), nullptr, nameBuffer);
            
            m_ActiveUBOs.push_back({ std::string(nameBuffer), values[0], values[1] });
        }

        // Sort UBOs by binding point for neatness
        std::sort(m_ActiveUBOs.begin(), m_ActiveUBOs.end(), [](const BufferBlockInfo& a, const BufferBlockInfo& b) {
            return a.BindingPoint < b.BindingPoint;
        });

        // --- Extract Shader Storage Blocks (SSBOs) ---
        int ssboCount = 0;
        glGetProgramInterfaceiv(id, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &ssboCount);

        for (int i = 0; i < ssboCount; i++) {
            // Fetch binding point and size
            glGetProgramResourceiv(id, GL_SHADER_STORAGE_BLOCK, i, 2, blockProperties, 2, nullptr, values);
            // Fetch the block name
            glGetProgramResourceName(id, GL_SHADER_STORAGE_BLOCK, i, sizeof(nameBuffer), nullptr, nameBuffer);
            
            m_ActiveSSBOs.push_back({ std::string(nameBuffer), values[0], values[1] });
        }

        // Sort SSBOs by binding point
        std::sort(m_ActiveSSBOs.begin(), m_ActiveSSBOs.end(), [](const BufferBlockInfo& a, const BufferBlockInfo& b) {
            return a.BindingPoint < b.BindingPoint;
        });
    }


    void OpenGLShader::Bind() const
    {
        glUseProgram(id);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }

    GLint OpenGLShader::GetUniformLocation(const std::string &name) const
    {
        auto it = m_UniformLocationCache.find(name);
        if (it != m_UniformLocationCache.end()) {
            return it->second;
        }

        return -1;
    }

    void OpenGLShader::SetBool(const std::string &name, bool value) const
    {         
        glUniform1i(GetUniformLocation(name.c_str()), (int)value); 
    }
    // ------------------------------------------------------------------------
    void OpenGLShader::SetInt(const std::string &name, int value) const
    { 
        glUniform1i(GetUniformLocation(name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void OpenGLShader::SetFloat(const std::string &name, float value) const
    { 
        glUniform1f(GetUniformLocation(name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void OpenGLShader::SetVec2(const std::string &name, const glm::vec2 &value) const
    { 
        glUniform2fv(GetUniformLocation(name.c_str()), 1, &value[0]); 
    }
    // ------------------------------------------------------------------------
    void OpenGLShader::SetVec3(const std::string &name, const glm::vec3 &value) const
    { 
        glUniform3fv(GetUniformLocation(name.c_str()), 1, &value[0]); 
    }
    // ------------------------------------------------------------------------
    void OpenGLShader::SetVec4(const std::string &name, const glm::vec4 &value) const
    { 
        glUniform4fv(GetUniformLocation(name.c_str()), 1, &value[0]); 
    }
    // ------------------------------------------------------------------------
    void OpenGLShader::SetMat2(const std::string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(GetUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void OpenGLShader::SetMat3(const std::string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(GetUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void OpenGLShader::SetMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(GetUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

};
