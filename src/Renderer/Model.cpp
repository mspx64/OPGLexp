#include <filesystem>
#include "Model.h"
#include "Scene.h"


static glm::mat4 AiToGlm(const aiMatrix4x4& from)
{
	// Assimp is row-major, GLM is column-major — transpose on conversion
	return glm::mat4(
		from.a1, from.b1, from.c1, from.d1,
		from.a2, from.b2, from.c2, from.d2,
		from.a3, from.b3, from.c3, from.d3,
		from.a4, from.b4, from.c4, from.d4);
}

// Shared Assimp import flags — aiProcess_PreTransformVertices is intentionally
// omitted so that node transforms are preserved and the gizmo can move meshes.
static constexpr unsigned int IMPORT_FLAGS =
aiProcess_Triangulate |
aiProcess_GenSmoothNormals |
aiProcess_CalcTangentSpace |
aiProcess_GlobalScale;


bool Model::loadFromFile(const std::string& filepath)
{
	m_TextureFilePath = std::filesystem::path(filepath).parent_path().string();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath, IMPORT_FLAGS);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG(LogLevel::_ERROR, "Failed to load model: " + filepath);
		LOG(LogLevel::_ERROR, "Assimp error: " + std::string(importer.GetErrorString()));
		return false;
	}

	LOG(LogLevel::DEBUG, "Model imported successfully: " + filepath);
	LOG(LogLevel::DEBUG, "Meshes: " + std::to_string(scene->mNumMeshes));

	// Pass identity as the root parent transform — each node accumulates downward
	processNode(scene->mRootNode, scene, glm::mat4(1.0f));

	LOG(LogLevel::DEBUG, "Model loaded: " + filepath +
		" | Nodes: " + std::to_string(m_Nodes.size()));
	return true;
}


Model::Model(const std::string& filepath) : m_ModelFilepath(filepath)
{
	loadFromFile(filepath);
}

Model::Model(const std::string& filepath, lgt::Scene* scene)
	: m_ModelFilepath(filepath)
{
	if (!loadFromFile(filepath) || !scene)
		return;

	// Populate the ECS scene — one entity per node that has meshes
	for (auto& node : m_Nodes)
	{
		if (node.meshes.empty())
			continue; // skip empty nodes (e.g. structural/helper nodes)

		Renderable component;
		component.Transform = node._transform;

		for (const auto& mesh : node.meshes)
			component._meshes.push_back(mesh);

		lgt::Entity e = scene->m_Roster->createEntity(node.name);
		e.addComponent<Renderable>(component);
		scene->m_Entites.push_back(e);
	}

	LOG(LogLevel::_INFO, "Scene populated with " +
		std::to_string(scene->m_Entites.size()) + " entities.");
}


void Model::cleanUp()
{
	LOG(LogLevel::DEBUG, "Destroying model: " + m_ModelFilepath);
	for (auto& mesh : m_Meshes)
		mesh.cleanUp();
}


void Model::processNode(const aiNode* node, const aiScene* scene, glm::mat4 parentTransform)
{
	// Accumulate the full world-space transform down the hierarchy.
	// Without this, nodes only store their local transform and meshes
	// appear at the wrong position unless PreTransformVertices is used.
	glm::mat4 worldTransform = parentTransform * AiToGlm(node->mTransformation);

	Node myNode;
	myNode.name = node->mName.C_Str();
	myNode._transform = worldTransform;

	LOG(LogLevel::_INFO,
		"Node: " + myNode.name +
		" | Meshes: " + std::to_string(node->mNumMeshes) +
		" | Children: " + std::to_string(node->mNumChildren));

	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		myNode.meshes.push_back(processMesh(mesh, scene));
	}

	m_Nodes.push_back(std::move(myNode));

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		processNode(node->mChildren[i], scene, worldTransform); // pass accumulated transform
}


Mesh Model::processMesh(const aiMesh* mesh, const aiScene* scene)
{
	std::vector<vertex>       Vertices;
	std::vector<unsigned int> Indices;
	std::vector<std::shared_ptr<Texture>> Textures;
	Material material;

	Vertices.reserve(mesh->mNumVertices);

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		vertex v;
		v.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

		v.norm = mesh->HasNormals()
			? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
			: glm::vec3(0.0f, 1.0f, 0.0f);

		v.textcoord = mesh->mTextureCoords[0]
			? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
			: glm::vec2(0.0f);

		if (mesh->HasTangentsAndBitangents())
		{
			v.tangent = { mesh->mTangents[i].x,   mesh->mTangents[i].y,   mesh->mTangents[i].z };
			v.bitangent = { mesh->mBitangents[i].x,  mesh->mBitangents[i].y, mesh->mBitangents[i].z };
		}
		else
		{
			v.tangent = { 1.0f, 0.0f, 0.0f };
			v.bitangent = { 0.0f, 0.0f, 1.0f };
		}

		Vertices.push_back(v);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
			Indices.push_back(face.mIndices[j]);
	}

	LOG(LogLevel::DEBUG, "Mesh loaded: " + std::string(mesh->mName.C_Str()) +
		" | Verts: " + std::to_string(Vertices.size()) +
		" | Indices: " + std::to_string(Indices.size()) +
		" | Tangents: " + (mesh->HasTangentsAndBitangents() ? "yes" : "no"));

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* M = scene->mMaterials[mesh->mMaterialIndex];
		material = LoadMaterial(M);

		struct TexTypeInfo { aiTextureType ai; TextureType mt; const char* label; };
		static const TexTypeInfo texTypes[] = {
			{ aiTextureType_DIFFUSE,  TextureType::DIFFUSE,  "diffuse"  },
			{ aiTextureType_NORMALS,  TextureType::NORMAL,   "normal"   },
			{ aiTextureType_SPECULAR, TextureType::SPECULAR, "specular" },
			{ aiTextureType_HEIGHT,   TextureType::NORMAL,   "height"   },
		};

		for (const auto& t : texTypes)
		{
			unsigned int count = M->GetTextureCount(t.ai);

			if (count > 0)
			{
				if (t.mt == TextureType::NORMAL)   material.hasNormalMap = true;
				if (t.mt == TextureType::SPECULAR)  material.hasSpecularMap = true;
			}

			for (unsigned int i = 0; i < count; ++i)
			{
				aiString str;
				M->GetTexture(t.ai, i, &str);
				std::string texPath = m_TextureFilePath + "/" + str.C_Str();

				auto tex = std::make_shared<Texture>(texPath);
				tex->setType(t.mt);
				Textures.push_back(tex);

				LOG(LogLevel::_INFO, std::string("Texture [") + t.label + "]: " + texPath);
			}
		}
	}

	return Mesh(Vertices, Indices, material, Textures);
}

Material Model::LoadMaterial(aiMaterial* M) const
{
	Material mat;
	aiColor3D color;
	float shininess = 0.0f;

	if (M->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) mat.diffuse = { color.r, color.g, color.b };
	if (M->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) mat.specular = { color.r, color.g, color.b };
	if (M->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) mat.ambient = { color.r, color.g, color.b };
	if (M->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) mat.shininess = shininess;

	return mat;
}

// Full-parameter overload (legacy path)
void Model::Render(const shader& Shader, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix,
	const glm::mat4& projectionMatrix, const glm::vec3& viewPos, const glm::vec3& lightPos,
	const glm::vec3& lightColor, bool useColor, const glm::vec3& color)
{
	Shader.setMat4("u_view", viewMatrix);
	Shader.setMat4("u_projection", projectionMatrix);

	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
	Shader.setMat3("u_normalMatrix", normalMatrix);

	Shader.setVec3("u_viewPos", viewPos);
	Shader.setVec3("u_light.position", lightPos);
	Shader.setVec3("u_light.color", lightColor);
	Shader.setFloat("u_light.intensity", 1.0f);
	Shader.setFloat("u_light.constant", 1.0f);
	Shader.setFloat("u_light.linear", 0.09f);
	Shader.setFloat("u_light.quadratic", 0.032f);
	Shader.setBool("u_useColor", useColor);
	if (useColor) Shader.setVec3("u_color", color);

	for (auto& mesh : m_Meshes)
		mesh.render(Shader);
}

// Convenience overload — renders all nodes using their world transforms.
// Each node sets its own u_model so the shader sees the correct matrix.
void Model::Render(const shader& Shader)
{
	for (auto& node : m_Nodes) // NOTE: by reference — avoids copying mesh/texture data
	{
		Shader.setMat4("u_model", node._transform);

		// Normal matrix derived from this node's world transform
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(node._transform)));
		Shader.setMat3("u_normalMatrix", normalMatrix);

		for (auto& mesh : node.meshes) // also by reference
			mesh.render(Shader);
	}
}