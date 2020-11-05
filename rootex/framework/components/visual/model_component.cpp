#include "model_component.h"

#include "common/common.h"

#include "core/resource_loader.h"
#include "event_manager.h"
#include "framework/entity.h"
#include "framework/systems/light_system.h"
#include "framework/systems/render_system.h"
#include "framework/components/visual/static_point_light_component.h"
#include "renderer/material_library.h"
#include "renderer/render_pass.h"

Component* ModelComponent::Create(const JSON::json& componentData)
{
	HashMap<String, String> materialOverrides;
	if (componentData.find("materialOverrides") != componentData.end())
	{
		for (auto& element : JSON::json::iterator_wrapper(componentData["materialOverrides"]))
		{
			materialOverrides[element.key()] = element.value();
		}
	}
	Vector<EntityID> affectingStaticLights;
	if (componentData.find("affectingStaticLights") != componentData.end())
	{
		for (int lightEntityID : componentData["affectingStaticLights"])
		{
			affectingStaticLights.push_back(lightEntityID);
		}
	}
	ModelComponent* modelComponent = new ModelComponent(
	    componentData["renderPass"],
	    ResourceLoader::CreateModelResourceFile(componentData["resFile"]),
	    materialOverrides,
	    componentData["isVisible"],
	    affectingStaticLights);

	return modelComponent;
}

Component* ModelComponent::CreateDefault()
{
	ModelComponent* modelComponent = new ModelComponent(
	    (int)RenderPass::Basic,
	    ResourceLoader::CreateModelResourceFile("rootex/assets/cube.obj"),
	    {},
	    true,
	    {});

	return modelComponent;
}

ModelComponent::ModelComponent(unsigned int renderPass, ModelResourceFile* resFile, const HashMap<String, String>& materialOverrides, bool visibility, const Vector<EntityID>& affectingStaticLightIDs)
    : RenderableComponent(renderPass, materialOverrides, visibility, affectingStaticLightIDs)
{
	setVisualModel(resFile, materialOverrides);
}

void ModelComponent::RegisterAPI(sol::table& rootex)
{
	sol::usertype<ModelComponent> modelComponent = rootex.new_usertype<ModelComponent>(
	    "ModelComponent",
	    sol::base_classes, sol::bases<Component>());
	rootex["Entity"]["getModel"] = &Entity::getComponent<ModelComponent>;
	modelComponent["isVisible"] = &ModelComponent::isVisible;
	modelComponent["setIsVisible"] = &ModelComponent::setIsVisible;
}

bool ModelComponent::compareMaterials(const Pair<Ref<Material>, Vector<Mesh>>& a, const Pair<Ref<Material>, Vector<Mesh>>& b)
{
	// Alpha materials final last
	return !a.first->isAlpha() && b.first->isAlpha();
}

void ModelComponent::render()
{
	std::sort(m_ModelResourceFile->getMeshes().begin(), m_ModelResourceFile->getMeshes().end(), compareMaterials);
	int i = 0;
	
	RenderableComponent::render();

	for (auto& [material, meshes] : m_ModelResourceFile->getMeshes())
	{
		RenderSystem::GetSingleton()->getRenderer()->bind(m_MaterialOverrides[material].get());
		i++;

		for (auto& mesh : meshes)
		{
			RenderSystem::GetSingleton()->getRenderer()->draw(mesh.m_VertexBuffer.get(), mesh.m_IndexBuffer.get());
		}
	}
}

void ModelComponent::setVisualModel(ModelResourceFile* newModel, const HashMap<String, String>& materialOverrides)
{
	if (!newModel)
	{
		return;
	}

	m_ModelResourceFile = newModel;
	m_MaterialOverrides.clear();
	for (auto& [material, meshes] : m_ModelResourceFile->getMeshes())
	{
		setMaterialOverride(material, material);
	}
	for (auto& [oldMaterial, newMaterial] : materialOverrides)
	{
		MaterialLibrary::CreateNewMaterialFile(newMaterial, MaterialLibrary::GetMaterial(oldMaterial)->getTypeName());
		setMaterialOverride(MaterialLibrary::GetMaterial(oldMaterial), MaterialLibrary::GetMaterial(newMaterial));
	}
}

JSON::json ModelComponent::getJSON() const
{
	JSON::json j = RenderableComponent::getJSON();

	j["resFile"] = m_ModelResourceFile->getPath().string();

	return j;
}

#ifdef ROOTEX_EDITOR
#include "imgui.h"
#include "imgui_stdlib.h"
void ModelComponent::draw()
{
	ImGui::Checkbox("Visible", &m_IsVisible);

	ImGui::BeginGroup();

	String inputPath = m_ModelResourceFile->getPath().generic_string();
	ImGui::InputText("##Path", &inputPath);
	ImGui::SameLine();
	if (ImGui::Button("Create Visual Model"))
	{
		if (!ResourceLoader::CreateModelResourceFile(inputPath))
		{
			WARN("Could not create Visual Model");
		}
		else
		{
			inputPath = "";
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Model"))
	{
		EventManager::GetSingleton()->call("OpenModel", "EditorOpenFile", m_ModelResourceFile->getPath().string());
	}
	ImGui::EndGroup();

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Resource Drop"))
		{
			const char* payloadFileName = (const char*)payload->Data;
			FilePath payloadPath(payloadFileName);
			if (IsFileSupported(payloadPath.extension().string(), ResourceFile::Type::Model))
			{
				setVisualModel(ResourceLoader::CreateModelResourceFile(payloadPath.string()), {});
			}
			else
			{
				WARN("Unsupported file format for Model");
			}
		}
		ImGui::EndDragDropTarget();
	}

	RenderableComponent::draw();
}
#endif // ROOTEX_EDITOR
