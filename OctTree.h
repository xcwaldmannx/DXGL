#pragma once

#include <array>
#include <vector>
#include <list>
#include <memory>

#include "Vec2f.h"
#include "Vec3f.h"
#include "Point2f.h"
#include "Point3f.h"

struct OctTreeRect {
	Vec3f pos;
	Vec3f size;

	bool contains(const OctTreeRect& rect) const {
		return rect.pos.x >= pos.x && rect.pos.x + rect.size.x < pos.x + size.x
			&& rect.pos.y >= pos.y && rect.pos.y + rect.size.y < pos.y + size.y
			&& rect.pos.z >= pos.z && rect.pos.z + rect.size.z < pos.z + size.z;
	}

	bool contains(const Vec3f& point) const {
		return point.x >= pos.x && point.x < pos.x + size.x
			&& point.y >= pos.y && point.y < pos.y + size.y
			&& point.z >= pos.z && point.z < pos.z + size.z;
	}

	bool overlaps(const OctTreeRect& rect) const {
		return rect.pos.x < pos.x + size.x && rect.pos.x + rect.size.x > pos.x
			&& rect.pos.y < pos.y + size.y && rect.pos.y + rect.size.y > pos.y
			&& rect.pos.z < pos.z + size.z && rect.pos.z + rect.size.z > pos.z;
	}
};

template<typename T>
class OctTreeWrap {
public:
	OctTreeWrap(const OctTreeRect& rect = { Vec3f{ 0, 0, 0 }, Vec3f{ 1, 1, 1 } }, const size_t maxDepth = 4) : m_rect(rect), MAX_DEPTH(maxDepth) {
	}

	~OctTreeWrap() {
	}

	void insert(const T& item, const OctTreeRect& rect) {
		for (int i = 0; i < 8; i++) {
			if (m_childRects[i].contains(rect)) {
				if (m_depth + 1 < MAX_DEPTH) {
					if (!m_children[i]) {
						m_children[i] = std::make_shared<OctTreeWrap<T>>(m_childRects[i], m_depth + 1);
					}

					m_children[i]->insert(item, rect);
					return;
				}
			}
		}

		m_items.push_back({ rect, item });
	}

	std::list<T> search(const OctTreeRect& rect) {
		std::list<T> items{};
		search(rect, items);
		return items;
	}

	void items(std::list<T>& items) {
		for (const auto& item : m_items) {
			items.push_back(item.second);

			for (int i = 0; i < 8; i++) {
				if (m_children[i]) m_children[i]->items(items);
			}
		}
	}

	void search(const OctTreeRect& rect, std::list<T>& items) {
		for (const auto& item : m_items) {
			if (rect.overlaps(item.first)) items.push_back(item.second);
		}

		for (int i = 0; i < 8; i++) {
			if (m_children[i]) {
				if (rect.contains(m_childRects[i])) {
					m_children[i]->items(items);
				} else if (m_childRects[i].overlaps(rect)) {
					m_children[i]->search(rect, items);
				}
			}
		}
	}

	void resize(const OctTreeRect& rect) {
		clear();
		m_rect = rect;

		Vec3f childSize = m_rect.size / 2.0f;

		m_childRects = {
			// lower (no z)
			OctTreeRect(m_rect.pos, childSize),
			OctTreeRect({ m_rect.pos.x + childSize.x, m_rect.pos.y, m_rect.pos.z }, childSize),
			OctTreeRect({ m_rect.pos.x, m_rect.pos.y + childSize.y, m_rect.pos.z }, childSize),
			OctTreeRect({ m_rect.pos.x + childSize.x, m_rect.pos.y + childSize.y, m_rect.pos.z }, childSize),
			// upper (use z)
			OctTreeRect({ m_rect.pos.x, m_rect.pos.y, m_rect.pos.z + childSize.z }, childSize),
			OctTreeRect({ m_rect.pos.x + childSize.x, m_rect.pos.y, m_rect.pos.z + childSize.z }, childSize),
			OctTreeRect({ m_rect.pos.x, m_rect.pos.y + childSize.y, m_rect.pos.z + childSize.z }, childSize),
			OctTreeRect(m_rect.pos + childSize, childSize),
		};
	}

	void clear() {
		m_items.clear();

		for (int i = 0; i < 8; i++) {
			if (m_children[i]) m_children[i]->clear();
			m_children[i].reset();
		}
	}

	size_t size() {
		size_t count = m_items.size();
		for (int i = 0; i < 8; i++) {
			if (m_children[i]) count += m_children[i]->size();
		}
		return count;
	}

private:
	OctTreeRect m_rect;

	size_t m_depth = 0;
	const size_t MAX_DEPTH;

	std::array<OctTreeRect, 8> m_childRects{};

	std::array<std::shared_ptr<OctTreeWrap<T>>, 8> m_children{};

	std::vector<std::pair<OctTreeRect, T>> m_items;
};

template<typename T>
class OctTree {

public:
	typedef std::list<T>::iterator ptr;
	typedef std::list<T>::const_iterator cptr;
	typedef std::list<typename ptr> list;

	OctTree(const OctTreeRect& rect = { Vec3f{0, 0, 0}, Vec3f{1, 1, 1} }, const size_t maxDepth = 4) : m_root(rect) {

	}

	void insert(const T& item, const OctTreeRect& rect) {
		m_items.push_back(item);

		m_root.insert(std::prev(m_items.end()), rect);
	}

	list search(const OctTreeRect& rect) {
		list items{};
		m_root.search(rect, items);
		return items;
	}

	void resize(const OctTreeRect& rect) {
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
	OctTreeWrap<typename ptr> m_root;
};