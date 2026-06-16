
#include "renderer.h"

VertexArray::VertexArray() {
    glGenVertexArrays(1, &m_RenderID);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_RenderID);
}
const void VertexArray::AddLayout(const VeretexLayout& layout, const VertexBuffer& vb) const

{
    size_t      offset   = 0;
    const auto& elements = layout.getElements();
    for (unsigned int i = 0; i < elements.size(); i++) {
        Bind();
        vb.Bind();
        const auto& element = elements[i];
        glEnableVertexAttribArray(i);
        GlCall(glVertexAttribPointer(i, element.count, element.type, element.normalize, layout.getStride(), (void*)offset));
        offset += element.count * sizeof(element.type);
    };
}

const void VertexArray::Bind() const {
    glBindVertexArray(m_RenderID);
}

const void VertexArray::Unbind() const {
    glBindVertexArray(0);
}
