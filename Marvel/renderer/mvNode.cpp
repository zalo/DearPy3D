#include "mvNode.h"
#include "mvMesh.h"
#include "mvModelProbe.h"

namespace Marvel {

	mvNode::mvNode(const std::string& name, int id, std::vector<std::shared_ptr<mvMesh>> meshPtrs, const glm::mat4& transform)
	{
		m_name = name;
		m_id = id;
		m_meshes = meshPtrs;
		m_transform = transform;
		m_appliedTransform = glm::identity<glm::mat4>();
	}

	int mvNode::getID() const
	{
		return m_id;
	}

	void mvNode::submit(mvRenderGraph& graph, glm::mat4 accumulatedTransform) const
	{
		const auto built = accumulatedTransform * m_transform * m_appliedTransform;

		for (const auto pm : m_meshes)
			pm->submit(graph, built);

		for (const auto& pc : m_children)
			pc->submit(graph, built);
	}

	void mvNode::accept(mvModelProbe& probe)
	{
		if (probe.pushNode(*this))
		{
			for (auto& cp : m_children)
				cp->accept(probe);
			probe.popNode(*this);
		}
	}

	void mvNode::draw(mvGraphics& graphics) const
	{
		for (const auto& pm : m_meshes)
			pm->draw(graphics);

		for (const auto& pc : m_children)
			pc->draw(graphics);
	}

	void mvNode::setAppliedTransform(glm::mat4 transform)
	{
		m_appliedTransform = transform;
	}

	const glm::mat4& mvNode::getAppliedTransform() const
	{
		return m_appliedTransform;
	}

	void mvNode::addChild(mvNode* child)
	{
		m_children.push_back(std::shared_ptr<mvNode>(child));
	}

	mvNode* mvNode::getNode(const std::string& name)
	{
		if (m_name == name)
			return this;

		for (const auto& child : m_children)
		{
			if (auto result = child->getNode(name))
				return result;
		}

		return nullptr;
	}

}