#pragma once

#include <vector>

#ifndef PX_PHYSX_STATIC_LIB
#define PX_PHYSX_STATIC_LIB
#endif

#include "PxPhysicsAPI.h"
#include "pvd/PxPvd.h"

#include "DXGLGovernor.h"
#include "OctTree.h"

using namespace physx;

namespace dxgl {

	class PhysicsManager {
	public:
		PhysicsManager();
		~PhysicsManager();

		void update(std::list<OctTree<governor::EntityId>::ptr>& entities, long double delta);

	private:
        PxDefaultAllocator m_allocator;
        PxDefaultErrorCallback m_errorCallback;
        PxFoundation* m_foundation = nullptr;
        PxPvd* m_pvd = nullptr;
        PxDefaultCpuDispatcher* m_dispatcher = nullptr;
        PxPhysics* m_physics = nullptr;
        PxScene* m_scene = nullptr;

        std::array<PxRigidActor*, MAX_ENTITIES> m_actors{};

        // cylindrical data for cylinder collider
        std::vector<Vec3f> m_cylinderVertices{};
        void generateCylinder(float segmentCount, float radius, float halfHeight);

        static PxFilterFlags testCCDFilterShader(
            PxFilterObjectAttributes attributes0,
            PxFilterData filterData0,
            PxFilterObjectAttributes attributes1,
            PxFilterData filterData1,
            PxPairFlags& pairFlags,
            const void* constantBlock,
            PxU32 constantBlockSize);

        void quatToEuler(const PxQuat& quat, float& pitch, float& yaw, float& roll) {
            float ysqr = quat.y * quat.y;

            // Pitch (x-axis rotation)
            float t0 = +2.0f * (quat.w * quat.x + quat.y * quat.z);
            float t1 = +1.0f - 2.0f * (quat.x * quat.x + ysqr);
            pitch = std::atan2(t0, t1);

            // Yaw (y-axis rotation)
            float t2 = +2.0f * (quat.w * quat.y - quat.z * quat.x);
            t2 = t2 > 1.0f ? 1.0f : t2;
            t2 = t2 < -1.0f ? -1.0f : t2;
            yaw = std::asin(t2);

            // Roll (z-axis rotation)
            float t3 = +2.0f * (quat.w * quat.z + quat.x * quat.y);
            float t4 = +1.0f - 2.0f * (ysqr + quat.z * quat.z);
            roll = std::atan2(t3, t4);
        }
	};
}