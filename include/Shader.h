#pragma once

#include "../pch.h"

class Shader
{
public:

    virtual ~Shader() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual const std::string& GetName() const = 0;

    static std::shared_ptr<Shader> Create(const std::string &name);
};

class ShaderLibrary
{
    public:
    void Add(const std::shared_ptr<Shader> &shader);

    std::shared_ptr<Shader> Get(const std::string &name);

    private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
};

