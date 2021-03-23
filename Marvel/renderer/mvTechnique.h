#pragma once

#include <vector>
#include <memory>
#include "mvStep.h"

namespace Marvel {

	class mvGraphics;
	class mvDrawable;
	class mvRenderGraph;

	class mvTechnique
	{

	public:

		void addStep(mvStep step);
		void submit(const mvDrawable& drawable) const;
		void setPass(mvRenderGraph& graph) const;

	private:

		std::vector<mvStep> m_steps;

	};


}