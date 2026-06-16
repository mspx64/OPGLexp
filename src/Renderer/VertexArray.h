#pragma once
#include "BufferLayout.h"
#include "VertexBuffer.h"

class VertexArray {
private:
    unsigned int m_RenderID;

public:
    VertexArray();
    ~VertexArray();
    const void AddLayout(const VeretexLayout& layout, const VertexBuffer& vb) const;
    const void Bind() const;
    const void Unbind() const;
};