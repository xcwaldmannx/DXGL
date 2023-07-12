#pragma once

#include <vector>

#include "DXGLMain.h"

#include "DXGLComponentTypes.h"

struct RenderQueueItem {
	governor::DXGLGroup entities{};
};

class RenderQueue {
public:
	RenderQueue();
	~RenderQueue();

	void submit(RenderQueueItem item);
	void process();

private:
	std::vector<RenderQueueItem> m_items{};
};