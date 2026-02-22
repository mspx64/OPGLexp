#pragma once
#include "Mesh.h"
#include "renderer.h"
#include "ecs/ECS.h"
#include <glm/gtx/string_cast.hpp>

struct Renderable
{
	std::vector<Mesh> _meshes;
	glm::mat4 Transform;
};
LGT_REGISTER_COMPONENT(lgt, Renderable);

namespace lgt
{
	class Scene
	{
	public:
		void Render(const shader& Shader)
		{
			for (auto& e : m_Entites)
			{
				auto& component = e.getComponent<Renderable>();
				Shader.setMat4("u_model", component.Transform);
				for (auto& mesh : component._meshes)
				{
					mesh.render(Shader);
				}
			}
		}

		const std::vector<Entity> getEntites()
		{
			return m_Entites;
		}
		void RenderScenePanel()
		{
			ImGui::Begin("Entities");
			if (m_Entites.empty())
			{
				ImGui::TextDisabled("No entities in scene");
			}
			else
			{
				for (auto& entity : m_Entites)
				{
					bool isSelected = (m_Selcted == entity.getHandle());
					if (ImGui::Selectable(entity.getName().c_str(), isSelected))
						m_Selcted = entity.getHandle();

					if (ImGui::BeginPopupContextItem(entity.getName().c_str()))
					{
						if (ImGui::MenuItem("Select"))
							m_Selcted = entity.getHandle();
						ImGui::EndPopup();
					}
				}
			}
			ImGui::End();
		}

		void RenderGizmo(glm::mat4 view, glm::mat4 proj, ImGuizmo::OPERATION operation,
			ImVec2 windowPos, ImVec2 windowSize)
		{
			if (m_Selcted == EntityHandle{}) return;

			for (auto& entity : m_Entites)
			{
				if (entity.getHandle() != m_Selcted) continue;

				auto& renderable = entity.getComponent<Renderable>();
				glm::mat4 before = renderable.Transform;

				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
				ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

				bool changed = ImGuizmo::Manipulate(
					glm::value_ptr(view),
					glm::value_ptr(proj),
					operation,
					ImGuizmo::LOCAL,
					glm::value_ptr(renderable.Transform)
				);

				if (changed)
				{
					LOG(LogLevel::_INFO, "Before: " + glm::to_string(before));
					LOG(LogLevel::_INFO, "After:  " + glm::to_string(renderable.Transform));
				}

				break;
			}
		}

		Scene()
		{
			m_Roster = std::make_unique<Roster>();
		}

	private:
		EntityHandle m_Selcted;
		Scope<Roster> m_Roster;
		std::vector<Entity> m_Entites;

		friend Model;
	};
}
