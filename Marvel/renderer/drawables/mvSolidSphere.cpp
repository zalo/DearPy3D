#include "mvSolidSphere.h"
#include <imgui.h>
#include "mvGraphics.h"
#include "mvCommonBindables.h"
#include "Sphere.h"
#include "mvTechnique.h"
#include "mvMarvelUtils.h"

namespace Marvel {

	mvSolidSphere::mvSolidSphere(mvGraphics& graphics, float radius, glm::vec3 color, int simple)
	{

		// create topology
		m_topology = std::make_shared<mvTopology>(graphics, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// create vertex layout
		mvVertexLayout vl;
		vl.append(ElementType::Position3D);
		vl.append(ElementType::Normal);

		std::vector<float> verticies;
		std::vector<unsigned int> indicies;
		std::vector<float> normals;
		Sphere sphere(radius);

		verticies = sphere.vertices;
		indicies = sphere.indices;
		normals = sphere.normals;

		std::vector<float> nverticies;
		for (int i = 0; i < verticies.size(); i = i + 3)
		{
			nverticies.push_back(verticies[i]);
			nverticies.push_back(verticies[i+1]);
			nverticies.push_back(verticies[i+2]);
			nverticies.push_back(normals[i]);
			nverticies.push_back(normals[i+1]);
			nverticies.push_back(normals[i+2]);
		}

		// create vertex buffer
		m_vertexBuffer = std::make_shared<mvVertexBuffer>(graphics, nverticies, vl);

		// create index buffer
		m_indexBuffer = std::make_shared<mvIndexBuffer>(graphics, indicies);

		mvStep step("lambertian");

		// solid color
		if (simple == 1)
		{
			mvBufferLayout layout(std::make_shared<mvBufferLayoutEntry>(Struct));
			auto& root = layout.getRoot();
			root->add(Float3, std::string("materialColor"));
			root->finalize(0);

			std::unique_ptr<mvBuffer> bufferRaw = std::make_unique<mvBuffer>(std::move(layout));

			bufferRaw->getElement("materialColor").setIfExists(color);

			std::shared_ptr<mvPixelConstantBuffer> buf = std::make_shared<mvPixelConstantBuffer>(graphics, *root.get(), 1, bufferRaw.get());

			step.addBindable(buf);

			// create vertex shader
			auto vshader = mvBindableRegistry::GetBindable("Solid_VS");
			step.addBindable(vshader);
			step.addBindable(std::make_shared<mvInputLayout>(graphics, vl,
				static_cast<mvVertexShader*>(vshader.get())));
			step.addBindable(mvBindableRegistry::GetBindable("Solid_PS"));
			step.addBindable(std::make_shared<mvNullGeometryShader>(graphics));
			step.addBindable(std::make_shared<mvTransformConstantBuffer>(graphics));
		}

		// flat shade
		else if(simple == 2)
		{
			// create vertex shader
			auto vshader = std::make_shared<mvVertexShader>(graphics, "../../Marvel/shaders/vs_flat.hlsl");
			step.addBindable(vshader);
			step.addBindable(std::make_shared<mvInputLayout>(graphics, vl,
				static_cast<mvVertexShader*>(vshader.get())));
			step.addBindable(std::make_shared<mvPixelShader>(graphics, "../../Marvel/shaders/ps_flat.hlsl"));
			step.addBindable(std::make_shared<mvGeometryShader>(graphics, "../../Marvel/shaders/gs_flat.hlsl"));
			step.addBindable(mvBindableRegistry::GetBindable("transCBuf"));
		}

		// phong
		else
		{
			mvBufferLayout layout(std::make_shared<mvBufferLayoutEntry>(Struct));
			auto& root = layout.getRoot();
			root->add(Float3, std::string("materialColor"));
			root->add(Float3, std::string("specularColor"));
			root->add(Float, std::string("specularWeight"));
			root->add(Float, std::string("specularGloss"));
			root->finalize(0);

			std::unique_ptr<mvBuffer> bufferRaw = std::make_unique<mvBuffer>(std::move(layout));

			bufferRaw->getElement("materialColor").setIfExists(glm::vec3{ 1.0f, 1.0f, 1.0f });
			bufferRaw->getElement("specularColor").setIfExists(glm::vec3{ 1.0f, 1.0f, 1.0f });
			bufferRaw->getElement("specularWeight").setIfExists(1.0f);
			bufferRaw->getElement("specularGloss").setIfExists(8.0f);

			std::shared_ptr<mvPixelConstantBuffer> buf = std::make_shared<mvPixelConstantBuffer>(graphics, *root.get(), 1, bufferRaw.get());

			step.addBindable(buf);

			// create vertex shader
			auto vshader = mvBindableRegistry::GetBindable("Phong_VS");
			step.addBindable(vshader);
			step.addBindable(std::make_shared<mvInputLayout>(graphics, vl,
				static_cast<mvVertexShader*>(vshader.get())));
			step.addBindable(mvBindableRegistry::GetBindable("Phong_PS"));
			step.addBindable(std::make_shared<mvNullGeometryShader>(graphics));
			step.addBindable(mvBindableRegistry::GetBindable("transCBuf"));
		}


		mvTechnique technique;
		technique.addStep(step);
		addTechnique(technique);

	}

	void mvSolidSphere::setTransform(glm::mat4 transform)
	{
		const auto angles = ExtractEulerAngles(transform);
		const auto translation = ExtractTranslation(transform);
		m_x = translation.x;
		m_y = translation.y;
		m_z = translation.z;
		m_xangle = angles.x;
		m_yangle = angles.y;
		m_zangle = angles.z;
	}

	glm::mat4 mvSolidSphere::getTransform() const
	{
		return glm::translate(glm::vec3{ m_x, m_y, m_z }) *
			glm::rotate(m_zangle, glm::vec3{ 0.0f, 0.0f, 1.0f }) *
			glm::rotate(m_yangle, glm::vec3{ 0.0f, 1.0f, 0.0f }) *
			glm::rotate(m_xangle, glm::vec3{ 1.0f, 0.0f, 0.0f });
	}

	void mvSolidSphere::show_imgui_windows(const char* name)
	{
		if (ImGui::Begin(name))
		{
			ImGui::SliderFloat("X-Pos", &m_x, -50.0f, 50.0f);
			ImGui::SliderFloat("Y-Pos", &m_y, -50.0f, 50.0f);
			ImGui::SliderFloat("Z-Pos", &m_z, -50.0f, 50.0f);
			ImGui::SliderFloat("X-Angle", &m_xangle, -50.0f, 50.0f);
			ImGui::SliderFloat("Y-Angle", &m_yangle, -50.0f, 50.0f);
			ImGui::SliderFloat("Z-Angle", &m_zangle, -50.0f, 50.0f);
			//ImGui::ColorEdit3("Material Color", &m_material->m_cbData.materialColor.x);
			//ImGui::ColorEdit3("Specular Color", &m_material->m_cbData.specularColor.x);
			//ImGui::SliderFloat("Specular Weight", &m_material->m_cbData.specularWeight, 0.0f, 100.0f);
			//ImGui::SliderFloat("Specular Gloss", &m_material->m_cbData.specularGloss, 0.0f, 100.0f);
		}
		ImGui::End();
	}

	void mvSolidSphere::setPosition(float x, float y, float z)
	{
		m_x = x;
		m_y = y;
		m_z = z;
	}

	void mvSolidSphere::setRotation(float x, float y, float z)
	{
		m_xangle = x;
		m_yangle = y;
		m_zangle = z;
	}

}