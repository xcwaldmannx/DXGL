#pragma once

#include <memory>
#include <cstdint>
#include <bitset>

#define MAX_ENTITIES 8192
#define NULL_ENTITY (int) -1
#define MAX_COMPONENTS 32
#define NULL_COMPONENT (int) -1

namespace dxgl::governor {

	typedef uint32_t EntityId;
	typedef std::bitset<MAX_COMPONENTS> Signature;
	typedef uint32_t ComponentId;

	enum GroupSort {
		GROUP_ONLY,
		GROUP_ANY,
		GROUP_NOT,
	};

}