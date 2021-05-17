#include "mvSkyboxPass.h"
#include "mvGraphics.h"
#include "mvCommonBindables.h"
#include "mvCommonBuffers.h"

namespace Marvel {

	mvSkyboxPass::mvSkyboxPass(mvGraphics& graphics, const std::string& name, const char* skybox)
		:
		mvPass(name)
	{
		addBindable(std::make_shared<mvCubeTexture>(graphics, skybox));
		addBindable(std::make_shared<mvStencil>(graphics, mvStencil::Mode::DepthFirst));
		addBindable(std::make_shared<mvSampler>(graphics, mvSampler::Type::Bilinear));
		addBindable(std::make_shared<mvRasterizer>(graphics, true));
		addBindable(std::make_shared<mvPixelShader>(graphics, std::string(graphics.getShaderRoot() + "Skybox_PS.hlsl").c_str()));
		auto vshader = std::make_shared<mvVertexShader>(graphics, std::string(graphics.getShaderRoot() + "Skybox_VS.hlsl").c_str());
		addBindable(vshader);
		addBindable(std::make_shared<mvTopology>(graphics, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		addBuffer(std::make_shared<mvSkyBoxTransformConstantBuffer>(graphics));

		requestResource(std::make_unique<mvBufferPassResource<mvRenderTarget>>("render_target", m_renderTarget));
		requestResource(std::make_unique<mvBufferPassResource<mvDepthStencil>>("depth_stencil", m_depthStencil));
		issueProduct(std::make_unique<mvBufferPassProduct<mvRenderTarget>>("render_target", m_renderTarget));
		issueProduct(std::make_unique<mvBufferPassProduct<mvDepthStencil>>("depth_stencil", m_depthStencil));

		// create vertex layout
		mvVertexLayout vl;
		vl.append(ElementType::Position3D);

		constexpr float side = 1.0f / 2.0f;

		// create vertex buffer
		m_vertexBuffer = std::make_shared<mvVertexBuffer>(graphics, name, std::vector<float>{
			-side, -side, -side,
				side, -side, -side,
				-side, side, -side,
				side, side, -side,
				-side, -side, side,
				side, -side, side,
				-side, side, side,
				side, side, side
		}, vl);

		// create index buffer
		m_indexBuffer = mvBufferRegistry::Request<mvIndexBuffer>(graphics, name, std::vector<unsigned int>{
			0, 2, 1, 2, 3, 1,
				1, 3, 5, 3, 7, 5,
				2, 6, 3, 3, 6, 7,
				4, 5, 7, 4, 7, 6,
				0, 4, 2, 2, 4, 6,
				0, 1, 4, 1, 5, 4
		}, false);

		addBindable(mvBindableRegistry::Request<mvInputLayout>(graphics, vl, *vshader));
	}

	void mvSkyboxPass::execute(mvGraphics& graphics) const
	{
		if (m_renderTarget)
			m_renderTarget->bindAsBuffer(graphics, m_depthStencil.get());
		else
			m_depthStencil->bindAsBuffer(graphics);

		m_vertexBuffer->bind(graphics);
		m_indexBuffer->bind(graphics);

		for (auto& buffer : m_buffers)
			buffer->bind(graphics);

		//m_passes[1].execute(graphics);
		for (auto& bind : m_bindables)
			bind->bind(graphics);
		graphics.drawIndexed(m_indexBuffer->getCount());
	}

}