#include "Mesh.h"

Mesh::Mesh(const std::vector<vertex>& data
	, const std::vector<unsigned int>& indices
	, const Material& material
	, std::vector<std::shared_ptr<Texture>> textures
) : m_indices(indices), _material(material), Textures(textures)
{
	// Create buffers
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ibo);
	glBindVertexArray(m_vao);

	// Upload vertex data
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(vertex), &data[0], GL_STATIC_DRAW);

	// Upload index data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Position attribute (location = 0)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);

	// Texture coordinates (location = 1)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, textcoord));

	// Normal (location = 2)
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, norm));

	// Tangent (location = 3)
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, tangent));

	// Bitangent (location = 4)
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, bitangent));

	glBindVertexArray(0);
}


void Mesh::render(const shader& shader)
{
	shader.use();
	switch (shader.getType())
	{
	case ShaderType::COLORSHADER:
	{

		for (int i = 0; i < Textures.size(); i++)
		{
			auto it = Textures[i];
			switch (it->getType())
			{
			case  TextureType::DIFFUSE: {
				it->Bind(i);
				break;
			};
			case  TextureType::NORMAL: {
				it->Bind(i);
				break;
			};
			case  TextureType::SPECULAR: {
				it->Bind(i);
				break;
			};
			case  TextureType::HEIGHT: {
				it->Bind(i);
				break;
			};
			case  TextureType::AMBIENT: {
				it->Bind(i);
				break;
			}
			default:
				break;
			}
		}
		break;
	}
	case ShaderType::DEPTHSHADER: {
		break;
	}
	default:
		break;
	}


	glBindVertexArray(m_vao);
	GlCall(glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr));
	glBindVertexArray(0);
}

void Mesh::setTransform(glm::mat4 transform)
{
	_transform = transform;
}

glm::mat4 Mesh::getTransform()
{
	return _transform;
}

void Mesh::cleanUp()
{
	LOG(LogLevel::DEBUG, "Dleting The Buffers");
	LOG(LogLevel::DEBUG, "VAO_ID->" + std::to_string(m_vao));
	LOG(LogLevel::DEBUG, "VBO_ID->" + std::to_string(m_vbo));
	LOG(LogLevel::DEBUG, "IBO_ID->" + std::to_string(m_ibo));
	glDeleteBuffers(1, &m_ibo);
	glDeleteBuffers(1, &m_vbo);
	glDeleteVertexArrays(1, &m_vao);
	LOG(LogLevel::DEBUG, "Buffers Deleted");
}
