#include "mvDrawable.h"
#include "mvContext.h"

namespace DearPy3D {

	void mvDrawable::cleanup()
	{
		vkDeviceWaitIdle(GetLogicalDevice());
		_deletionQueue.flush();
	}

	void mvDrawable::bind() const
	{
		_indexBuffer->bind();
		_vertexBuffer->bind();
	}

	uint32_t mvDrawable::getVertexCount() const
	{
		return _indexBuffer->getVertexCount();
	}

}