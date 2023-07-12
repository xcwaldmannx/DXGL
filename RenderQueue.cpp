#include "RenderQueue.h"

RenderQueue::RenderQueue() {

}

RenderQueue::~RenderQueue() {

}

void RenderQueue::submit(RenderQueueItem item) {
	m_items.push_back(item);
}

void RenderQueue::process() {
	for (RenderQueueItem item : m_items) {

	}
}
