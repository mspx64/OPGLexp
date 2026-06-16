#pragma once
#include "renderer.h"
#include <vector>

struct LayoutElements {
    unsigned int count;
    unsigned int type;
    unsigned int normalize;
};

class VeretexLayout {
private:
    std::vector<LayoutElements> m_elements;
    unsigned int                m_stride = 0;

public:
    template <typename T> void push(unsigned int count) { static_assert(false); }
    template <> void           push<float>(unsigned int count) {
        m_elements.push_back({count, GL_FLOAT, 0});
        m_stride += count * sizeof(GL_FLOAT);
    }

    template <> void push<unsigned int>(unsigned int count) {
        m_elements.push_back({count, GL_UNSIGNED_INT, 0});
        m_stride += count * sizeof(GL_UNSIGNED_INT);
    }

    template <> void push<char>(unsigned int count) {
        m_elements.push_back({count, GL_BYTE, 1});
        m_stride += count * sizeof(GL_BYTE);
    }

    inline const std::vector<LayoutElements>& getElements() const& { return m_elements; }
    inline const unsigned int                 getStride() const& { return m_stride; }
};