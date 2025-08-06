#pragma once

class IndexBuffer{
    public:
    ~IndexBuffer();
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual unsigned int GetCount() const = 0;
    
    static std::shared_ptr<IndexBuffer> Create(unsigned int* indices, unsigned int count);
};
