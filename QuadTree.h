#pragma once

#include <array>
#include <vector>
#include <unordered_map>
#include <list>
#include <memory>

#include "Vec2f.h"
#include "Point2f.h"

struct QuadTreeRect {
	Vec2f pos;
	Vec2f size;

	bool contains(const QuadTreeRect& rect) const {
		return rect.pos.x >= pos.x && rect.pos.x + rect.size.x < pos.x + size.x
			&& rect.pos.y >= pos.y && rect.pos.y + rect.size.y < pos.y + size.y;
	}

	bool contains(const Point2f& point) const {
		return point.x >= pos.x && point.x < pos.x + size.x
			&& point.y >= pos.y && point.y < pos.y + size.y;
	}

	bool overlaps(const QuadTreeRect& rect) const {
		return rect.pos.x < pos.x + size.x && rect.pos.x + rect.size.x > pos.x
			&& rect.pos.y < pos.y + size.y && rect.pos.y + rect.size.y > pos.y;
	}
};

template<typename T>
class QuadTreeWrap {
public:
	QuadTreeWrap(const QuadTreeRect& rect = { Vec2f{0, 0}, Vec2f{1, 1} },
		const size_t maxSize = 1024, const size_t maxDepth = 4) : m_rect(rect), MAX_SIZE(maxSize), MAX_DEPTH(maxDepth) {
		//m_items = std::array<std::pair<QuadTreeRect, T>, MAX_SIZE>;
	}

	~QuadTreeWrap() {
	}

	void insert(const T& item, const QuadTreeRect& rect) {
		for (int i = 0; i < 4; i++) {
			if (m_childRects[i].contains(rect)) {
				if (m_depth + 1 < MAX_DEPTH) {
					if (!m_children[i]) {
						m_children[i] = std::make_shared<QuadTreeWrap<T>>(m_childRects[i], m_depth + 1);
					}

					m_children[i]->insert(item, rect);
					return;
				}
			}
		}

		//m_itemToIndex[item] = m_items.size();
		m_items.push_back({ rect, item });
	}

	//void remove(const T& item) {
	//	for (int i = 0; i < 4; i++) {
	//		if (m_children[i]) {
	//			if (m_itemToIndex.find(item) != m_itemToIndex.end()) {
	//				unsigned int index = m_itemToIndex[item];
	//				m_items.erase(index);
	//				m_itemToIndex.erase(item);
	//				return;
	//			}
	//			else {
	//				m_children[i]->remove(item);
	//			}
	//		}
	//	}
	//}

	bool contains(const T& item) {
		return false;// m_itemToIndex.find(item) != m_itemToIndex.end();
	}

	std::list<T> search(const QuadTreeRect& rect) {
		std::list<T> items{};
		search(rect, items);
		return items;
	}

	void items(std::list<T>& items) {
		for (const auto& item : m_items) {
			items.push_back(item.second);

			for (int i = 0; i < 4; i++) {
				if (m_children[i]) m_children[i]->items(items);
			}
		}
	}

	void search(const QuadTreeRect& rect, std::list<T>& items) {
		for (const auto& item : m_items) {
			if (rect.overlaps(item.first)) items.push_back(item.second);
		}

		for (int i = 0; i < 4; i++) {
			if (m_children[i]) {
				if (rect.contains(m_childRects[i])) {
					m_children[i]->items(items);
				} else if (m_childRects[i].overlaps(rect)) {
					m_children[i]->search(rect, items);
				}
			}
		}
	}

	void resize(const QuadTreeRect& rect) {
		clear();
		m_rect = rect;

		Vec2f childSize = m_rect.size / 2.0f;

		m_childRects = {
			QuadTreeRect(m_rect.pos, childSize),
			QuadTreeRect({m_rect.pos.x + childSize.x, m_rect.pos.y }, childSize),
			QuadTreeRect({m_rect.pos.x, m_rect.pos.y + childSize.y }, childSize),
			QuadTreeRect(m_rect.pos + childSize, childSize),
		};
	}

	void clear() {
		m_items.clear();

		for (int i = 0; i < 4; i++) {
			if (m_children[i]) m_children[i]->clear();
			m_children[i].reset();
		}
	}

	size_t size() {
		size_t count = m_items.size();
		for (int i = 0; i < 4; i++) {
			if (m_children[i]) count += m_children[i]->size();
		}
		return count;
	}

private:
	QuadTreeRect m_rect;

	size_t m_depth = 0;
	const size_t MAX_DEPTH;

	const size_t MAX_SIZE;

	std::array<QuadTreeRect, 4> m_childRects{};

	std::array<std::shared_ptr<QuadTreeWrap<T>>, 4> m_children{};

	std::vector<std::pair<QuadTreeRect, T>> m_items;
	// std::unordered_map<T, unsigned int> m_itemToIndex;
};

template<typename T>
class QuadTree {

public:
	typedef std::list<T>::iterator ptr;
	typedef std::list<T>::const_iterator cptr;
	typedef std::list<typename ptr> list;

	QuadTree(const QuadTreeRect& rect = { Vec2f{0, 0}, Vec2f{1, 1} }, const size_t maxDepth = 4) : m_root(rect) {

	}

	void insert(const T& item, const QuadTreeRect& rect) {
		m_items.push_back(item);

		m_root.insert(std::prev(m_items.end()), rect);
	}

	//void remove(const T& item) {
	//	m_root.remove(item);
	//}

	list search(const QuadTreeRect& rect) {
		list items{};
		m_root.search(rect, items);
		return items;
	}

	void resize(const QuadTreeRect& rect) {
		m_root.resize(rect);
	}

	void clear() {
		m_root.clear();
		m_items.clear();
	}

	size_t size() {
		return m_items.size();
	}

	bool empty() {
		return m_items.empty();
	}

	std::list<T> allItems() {
		return m_items;
	}
	
	typename ptr begin() {
		return m_items.begin();
	}

	typename ptr end() {
		return m_items.end();
	}

	typename cptr cbegin() const {
		return m_items.cbegin();
	}

	typename cptr cend() const {
		return m_items.cend();
	}

protected:
	std::list<T> m_items{};
	QuadTreeWrap<typename ptr> m_root;
};