#include "mvCube.h"
#include "mvGraphics.h"
#include "mvTexture.h"
#include "mvSampler.h"
#include <chrono>

namespace DearPy3D {

	mvCube::mvCube(mvGraphics& graphics, const std::string& path)
	{

		_transformBuffer = std::make_shared<mvTransformUniform>(graphics);
		_transformBuffer->_parent = this;

		auto vlayout = mvVertexLayout();
		vlayout.append(ElementType::Position3D);
		vlayout.append(ElementType::Normal);
		vlayout.append(ElementType::Tangent);
		vlayout.append(ElementType::Bitangent);
		vlayout.append(ElementType::Texture2D);

		static const float side = 1.0f;
		auto vertices = std::vector<float>{
			-side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 0 near side
			 side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 1
			-side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 2
			 side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 3
			-side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 4 far side
			 side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 5
			-side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 6
			 side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 7
			-side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 8 left side
			-side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 9
			-side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 10
			-side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 11
			 side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 12 right side
			 side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 13
			 side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 14
			 side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 15
			-side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 16 bottom side
			 side, -side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 17
			-side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 18
			 side, -side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 19
			-side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // 20 top side
			 side,  side, -side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // 21
			-side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 22
			 side,  side,  side, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f   // 23
		};

		auto indices = std::vector<uint16_t>{
			 1,  2,  0,  1,  3,  2,
			 7,  5,  4,  6,  7,  4,
			 9, 10,  8,  9, 11, 10,
			15, 13, 12, 14, 15, 12,
			18, 17, 16, 19, 17, 18,
			21, 23, 20, 23, 22, 20
		};

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			auto v0 = vertices[14 * indices[i]];
			auto v1 = vertices[14 * indices[i + 1]];
			auto v2 = vertices[14 * indices[i + 2]];
			const auto p0 = glm::vec3{ v0, vertices[14 * indices[i] + 1], vertices[14 * indices[i] + 2] };
			const auto p1 = glm::vec3{ v1, vertices[14 * indices[i + 1] + 1], vertices[14 * indices[i + 1] + 2] };
			const auto p2 = glm::vec3{ v2, vertices[14 * indices[i + 2] + 1], vertices[14 * indices[i + 2] + 2] };

			const auto n = glm::normalize(glm::cross(p1 - p0, p2 - p0));
			vertices[14 * indices[i] + 3] = n[0];
			vertices[14 * indices[i] + 4] = n[1];
			vertices[14 * indices[i] + 5] = n[2];
			vertices[14 * indices[i + 1] + 3] = n[0];
			vertices[14 * indices[i + 1] + 4] = n[1];
			vertices[14 * indices[i + 1] + 5] = n[2];
			vertices[14 * indices[i + 2] + 3] = n[0];
			vertices[14 * indices[i + 2] + 4] = n[1];
			vertices[14 * indices[i + 2] + 5] = n[2];
		}

		_vertexBuffer = std::make_shared<mvVertexBuffer>(graphics, vertices);
		_indexBuffer = std::make_shared<mvIndexBuffer>(graphics, indices);
		_sampler = std::make_shared<mvSampler>(graphics);
		_texture = std::make_shared<mvTexture>(graphics, path);

		_descriptorSetLayout = std::make_shared<mvDescriptorSetLayout>();
		_descriptorSetLayout->append(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		_descriptorSetLayout->append(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		_descriptorSetLayout->finalize(graphics);

		for (int i = 0; i < 3; i++)
		{
			_descriptorSets.push_back(std::make_shared<mvDescriptorSet>());
			graphics.allocateDescriptorSet(&(*_descriptorSets.back()), *_descriptorSetLayout);
			_descriptorSets.back()->update(graphics, *_transformBuffer->_buf[i], _texture->getImageView(), _sampler->getSampler());
		}

		_pipeline = std::make_shared<mvPipeline>();
		_pipeline->setVertexLayout(vlayout);
		_pipeline->setVertexShader(graphics, "../../DearPy3D/shaders/vert.spv");
		_pipeline->setFragmentShader(graphics, "../../DearPy3D/shaders/frag.spv");
		_pipeline->setDescriptorSetLayout(_descriptorSetLayout);
		_pipeline->setDescriptorSets(_descriptorSets);
		_pipeline->finalize(graphics);

		_deletionQueue.pushDeletor([=, &graphics]() {
			_sampler->cleanup(graphics);
			_texture->cleanup(graphics);
			_descriptorSetLayout->cleanup(graphics);
			_indexBuffer->cleanup(graphics);
			_vertexBuffer->cleanup(graphics);
			_transformBuffer->cleanup(graphics);
			});
	}

	glm::mat4 mvCube::getTransform() const
	{
		return glm::translate(glm::vec3{ m_x, m_y, m_z }) *
			glm::rotate(m_xangle, glm::vec3{ 1.0f, 0.0f, 0.0f }) *
			glm::rotate(m_yangle, glm::vec3{ 0.0f, 1.0f, 0.0f }) *
			glm::rotate(m_zangle, glm::vec3{ 0.0f, 0.0f, 1.0f });
	}

	void mvCube::setPosition(float x, float y, float z)
	{
		m_x = x;
		m_y = y;
		m_z = z;
	}

	void mvCube::setRotation(float x, float y, float z)
	{
		m_xangle = x;
		m_yangle = y;
		m_zangle = z;
	}

	void mvCube::bind(mvGraphics& graphics) const
	{
		auto index = graphics.getCurrentImageIndex();
		_transformBuffer->bind(graphics);
		_descriptorSets[index]->bind(graphics, *_pipeline);
		_pipeline->bind(graphics);
		_indexBuffer->bind(graphics);
		_vertexBuffer->bind(graphics);
	}

}