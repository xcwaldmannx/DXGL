#pragma once

#include <vector>

#ifndef PX_PHYSX_STATIC_LIB
#define PX_PHYSX_STATIC_LIB
#endif

#include "PxPhysicsAPI.h"
#include <pvd/PxPvd.h>

#include "DXGLMain.h"
#include "OctTree.h"
#include "OOBB.h"

using namespace physx;

namespace dxgl {

	class PhysicsManager {
	public:
		PhysicsManager();
		~PhysicsManager();

		void update(std::list<OctTree<governor::EntityId>::ptr>& entities, long double delta);

	private:
		void detectCollision(std::vector<governor::EntityId>& entities);
		void respondCollision(std::vector<governor::EntityId>& entities, long double delta);

	private:
        PxDefaultAllocator m_allocator;
        PxDefaultErrorCallback m_errorCallback;
        PxFoundation* m_foundation = nullptr;
        PxPvd* m_pvd = nullptr;
        PxDefaultCpuDispatcher* m_dispatcher = nullptr;
        PxPhysics* m_physics = nullptr;
        PxScene* m_scene = nullptr;
        PxMaterial* m_material = nullptr;

        std::array<int, MAX_ENTITIES> m_idToActor;
        std::array<PxRigidActor*, MAX_ENTITIES> m_actors;

		std::vector<governor::EntityId> m_detected;

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

        bool intersectLineSegmentTriangle(const Vec3f& p0, const Vec3f& p1,
            const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, Vec3f& intersection) {
            Vec3f dir = p1 - p0;
            float t, u, v;

            Vec3f edge1 = v1 - v0;
            Vec3f edge2 = v2 - v0;
            Vec3f h = Vec3f::cross(dir, edge2);
            float a = Vec3f::dot(edge1, h);

            if (a > -0.00001f && a < 0.00001f)
                return false;

            float f = 1.0f / a;
            Vec3f s = p0 - v0;
            u = f * Vec3f::dot(s, h);

            if (u < 0.0f || u > 1.0f)
                return false;

            Vec3f q = Vec3f::cross(s, edge1);
            v = f * Vec3f::dot(dir, q);

            if (v < 0.0f || u + v > 1.0f)
                return false;

            t = f * Vec3f::dot(edge2, q);

            if (t > 0.00001f && t < 1.0f) {
                intersection = p0 + dir * t;
                return true;
            }

            return false;
        }

	};
}