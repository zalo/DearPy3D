#pragma once
#include <filesystem>
#include <memory>
#include "mvDrawable.h"
#include "mvMaterial.h"

namespace Marvel {

	//-----------------------------------------------------------------------------
	// forward declarations
	//-----------------------------------------------------------------------------
	class mvGraphics;
	class mvRenderGraph;
	struct mvObjMesh;
	struct mvObjMaterial;

	//-----------------------------------------------------------------------------
	// mvMesh
	//-----------------------------------------------------------------------------
	class mvMesh : public mvDrawable
	{

	public:

		mvMesh(mvGraphics& graphics, const std::string& name, const mvObjMesh& mesh,
			const mvObjMaterial& material, const std::filesystem::path& path, float scale = 1.0f, bool PBR = false);

		// propagates through graph submitting jobs
		// mesh -> technique -> step -> pass
		void submit(mvRenderGraph& graph, glm::mat4 accumulatedTranform) const;
		
		glm::mat4 getTransform() const override;

		void show_imgui_controls();

	private:

		mutable glm::mat4 m_transform = glm::identity<glm::mat4>();
		std::shared_ptr<mvMaterial> m_material = nullptr;

	};

}