#include "../include/shader.h"

Shader::Shader()
{
    id = 0;
}

void Shader::setup()
{}
void Shader::draw()
{}
void Shader::cleanup()
{}

Shader::~Shader()
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


bool Shader::Init()
{
    id = glCreateProgram();

    if (id == 0) {
        fprintf(stderr, "Error creating shader program\n");
        return false;
    }

    return true;
}

// Use this method to add shaders to the program. When finished - call finalize()
bool Shader::AddShader(GLenum ShaderType, const char* filename)
{
    std::string s;
    std::ifstream f;

    f.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    try 
    {
        // open files
        f.open(filename);
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
        fprintf(stderr, "Error compiling '%s': '%s'\n", filename, InfoLog);
        return false;
    }

    glAttachShader(id, ShaderObj);

    return true;
}


// After all the shaders have been added to the program call this function
// to link and validate the program.
bool Shader::Finalize()
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

    glValidateProgram(id);

    glGetProgramiv(id, GL_VALIDATE_STATUS, &Success);

    if (Success == 0) {
        glGetProgramInfoLog(id, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        return false;
    }

    // Delete the intermediate shader objects that have been added to the program
    for (std::list<GLuint>::iterator it = shaderObjList.begin() ; it != shaderObjList.end() ; it++) {
        glDeleteShader(*it);
    }

    shaderObjList.clear();

//    PrintUniformList();

    return true;
}


void Shader::PrintUniformList()
{
    int Count = 0;
    glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &Count);
    printf("Active Uniforms: %d\n", Count);

    GLint Size; 
    GLenum Type;
    const GLsizei BufSize = 16; 
    GLchar Name[BufSize];
    GLsizei Length; 

    for (int i = 0; i < Count; i++) {
        glGetActiveUniform(id, (GLuint)i, BufSize, &Length, &Size, &Type, Name);

        printf("Uniform #%d Type: %u Name: %s\n", i, Type, Name);
    }
}


void Shader::Enable()
{
    glUseProgram(id);
}


GLint Shader::GetUniformLocation(const char* pUniformName)
{
    return glGetUniformLocation(id, pUniformName);
}

void Shader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value); 
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(id, name.c_str()), value); 
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(id, name.c_str()), value); 
}
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{ 
    glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{ 
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
}
// ------------------------------------------------------------------------
void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{ 
    glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
