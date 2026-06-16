#pragma once
class IndexBuffer {

private:
    unsigned int m_count;
    unsigned int m_RenderID;

public:
    IndexBuffer(const unsigned int* data, unsigned int count);
    ~IndexBuffer();
    void                Bind() const;
    void                Unbind() const;
    inline unsigned int GetCount() const { return m_count; }
};