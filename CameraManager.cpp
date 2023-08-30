#include "CameraManager.h"

#include "Engine.h"
#include "InputManager.h"
#include "EntityManager.h"

#include "Mesh.h"

using namespace dxgl;

CameraManager::CameraManager() {
	Engine::entities()->onAddComponent<CameraComponent>([&](governor::EntityId id) {
        auto& camera = Engine::entities()->getEntityComponent<CameraComponent>(id);
        camera.mouseRotX = camera.rotation.x;
        camera.mouseRotY = camera.rotation.y;
	});
}

CameraManager::~CameraManager() {
}

void CameraManager::update(long double delta) {
	if (m_activeCameraId == -1) return;

	auto& transform = Engine::entities()->getEntityComponent<TransformComponent>(m_activeCameraId);
	auto& camera = Engine::entities()->getEntityComponent<CameraComponent>(m_activeCameraId);

    camera.worldMatrix.setIdentity();

	if (camera.trackMouse) {
		Point2f mouseDelta = Engine::input()->getMouseDelta();

		camera.mouseRotX += mouseDelta.y * (float) delta / 5.0f;
		camera.mouseRotY += mouseDelta.x * (float) delta / 5.0f;

        if (camera.mouseRotX >= 1.57f) {
            camera.mouseRotX = 1.57f;
        }  else if (camera.mouseRotX <= -1.57f) {
            camera.mouseRotX = -1.57f;
        }

        Mat4f rot{};
        rot.setIdentity();
        rot.setRotationX(camera.mouseRotX);
        camera.worldMatrix *= rot;
        rot.setIdentity();
        rot.setRotationY(camera.mouseRotY);
        camera.worldMatrix *= rot;
    }
    
    if (camera.trackEntity) {
        Mat4f rot{};

        rot.setIdentity();
        rot.setRotationX(transform.rotation.x);
        camera.worldMatrix *= rot;
        rot.setIdentity();
        rot.setRotationY(transform.rotation.y);
        camera.worldMatrix *= rot;
        rot.setIdentity();
        rot.setRotationZ(transform.rotation.z);
        camera.worldMatrix *= rot;
    }

    camera.translation = transform.translation;
	camera.worldMatrix.setTranslation(camera.translation);

	Mat4f inverseWorld = camera.worldMatrix;
	inverseWorld.inverse();
	camera.viewMatrix = inverseWorld;

	RECT window = Engine::window()->getWindowSize();
	int widthAspect = window.right;
	int heightAspect = window.bottom;
	float aspect = (float)widthAspect / (float)heightAspect;
	camera.projMatrix.setPerspective(1.2f, aspect, 0.1f, 5000.0f);
}

bool CameraManager::cullActiveCamera(governor::EntityId id) {
	auto& transform = Engine::entities()->getEntityComponent<TransformComponent>(id);
	auto& mesh = Engine::entities()->getEntityComponent<MeshComponent>(id);

    Vec3f position = transform.translation;

    AABB aabb = mesh.mesh->getAABB();
    Vec3f minVertex = aabb.min * transform.scale;
    Vec3f maxVertex = aabb.max * transform.scale;

    std::vector<Vec4f> planes = getFrustumPlanes();

    for (int i = 0; i < planes.size(); i++) {

        Vec4f planeNormal = Vec4f(planes[i].x, planes[i].y, planes[i].z, 0.0f);
        float planeConstant = planes[i].w;

        Vec3f axisVertex{};

        // x-axis
        if (planes[i].x < 0.0f) { // Which AABB vertex is furthest down (plane normals direction) the x axis
            axisVertex.x = minVertex.x + position.x; // min x plus tree positions x
        }
        else {
            axisVertex.x = maxVertex.x + position.x; // max x plus tree positions x
        }

        // y-axis
        if (planes[i].y < 0.0f) { // Which AABB vertex is furthest down (plane normals direction) the y axis
            axisVertex.y = minVertex.y + position.y; // min y plus tree positions y
        }
        else {
            axisVertex.y = maxVertex.y + position.y; // max y plus tree positions y
        }

        // z-axis
        if (planes[i].z < 0.0f) { // Which AABB vertex is furthest down (plane normals direction) the z axis
            axisVertex.z = minVertex.z + position.z; // min z plus tree positions z
        }
        else {
            axisVertex.z = maxVertex.z + position.z; // max z plus tree positions z
        }

        // Now we get the signed distance from the AABB vertex that's furthest down the frustum planes normal,
        // and if the signed distance is negative, then the entire bounding box is behind the frustum plane, which means
        // that it should be culled
        if (Vec3f::dot(planeNormal.xyz(), axisVertex) + planeConstant < 0.0f) {
            return true;
        }
    }

    return false;

}

bool CameraManager::cullActiveCamera(Vec3f position, Vec3f scale, Vec3f min, Vec3f max) {
    std::vector<Vec4f> planes = getFrustumPlanes();

    for (int i = 0; i < planes.size(); i++) {

        Vec4f planeNormal = Vec4f(planes[i].x, planes[i].y, planes[i].z, 0.0f);
        float planeConstant = planes[i].w;

        Vec3f axisVertex{};

        // x-axis
        if (planes[i].x < 0.0f) { // Which AABB vertex is furthest down (plane normals direction) the x axis
            axisVertex.x = min.x * scale.x + position.x; // min x plus tree positions x
        }
        else {
            axisVertex.x = max.x * scale.x + position.x; // max x plus tree positions x
        }

        // y-axis
        if (planes[i].y < 0.0f) { // Which AABB vertex is furthest down (plane normals direction) the y axis
            axisVertex.y = min.y * scale.y + position.y; // min y plus tree positions y
        }
        else {
            axisVertex.y = max.y * scale.y + position.y; // max y plus tree positions y
        }

        // z-axis
        if (planes[i].z < 0.0f) { // Which AABB vertex is furthest down (plane normals direction) the z axis
            axisVertex.z = min.z * scale.z + position.z; // min z plus tree positions z
        }
        else {
            axisVertex.z = max.z * scale.z + position.z; // max z plus tree positions z
        }

        // Now we get the signed distance from the AABB vertex that's furthest down the frustum planes normal,
        // and if the signed distance is negative, then the entire bounding box is behind the frustum plane, which means
        // that it should be culled
        if (Vec3f::dot(planeNormal.xyz(), axisVertex) + planeConstant < 0.0f) {
            return true;
        }
    }

    return false;
}

CameraComponent& CameraManager::getActiveCamera() {
    assert(m_activeCameraId != -1 && "No camera is currently active. Could not return CameraComponent.");
	return Engine::entities()->getEntityComponent<CameraComponent>(m_activeCameraId);
}

void CameraManager::setActiveCamera(governor::EntityId id) {
	if (Engine::entities()->entityHasComponent<CameraComponent>(id)) {

		if (m_activeCameraId != -1) {
			auto& camera = Engine::entities()->getEntityComponent<CameraComponent>(m_activeCameraId);
			camera.isActive = false;
		}

		auto& camera = Engine::entities()->getEntityComponent<CameraComponent>(id);
		m_activeCameraId = id;
		camera.isActive = true;
	}
}

std::vector<Vec4f> CameraManager::getFrustumPlanes() {
    auto& camera = Engine::entities()->getEntityComponent<CameraComponent>(m_activeCameraId);
    std::vector<Vec4f> frustumPlanes{};
    Mat4f viewProj = camera.viewMatrix * camera.projMatrix;

    Vec4f left{};
    left.x = viewProj.mat[0][3] + viewProj.mat[0][0];
    left.y = viewProj.mat[1][3] + viewProj.mat[1][0];
    left.z = viewProj.mat[2][3] + viewProj.mat[2][0];
    left.w = viewProj.mat[3][3] + viewProj.mat[3][0];
    frustumPlanes.push_back(left);

    Vec4f right{};
    right.x = viewProj.mat[0][3] - viewProj.mat[0][0];
    right.y = viewProj.mat[1][3] - viewProj.mat[1][0];
    right.z = viewProj.mat[2][3] - viewProj.mat[2][0];
    right.w = viewProj.mat[3][3] - viewProj.mat[3][0];
    frustumPlanes.push_back(right);

    Vec4f top{};
    top.x = viewProj.mat[0][3] - viewProj.mat[0][1];
    top.y = viewProj.mat[1][3] - viewProj.mat[1][1];
    top.z = viewProj.mat[2][3] - viewProj.mat[2][1];
    top.w = viewProj.mat[3][3] - viewProj.mat[3][1];
    frustumPlanes.push_back(top);

    Vec4f bottom{};
    bottom.x = viewProj.mat[0][3] + viewProj.mat[0][1];
    bottom.y = viewProj.mat[1][3] + viewProj.mat[1][1];
    bottom.z = viewProj.mat[2][3] + viewProj.mat[2][1];
    bottom.w = viewProj.mat[3][3] + viewProj.mat[3][1];
    frustumPlanes.push_back(bottom);

    Vec4f nearr{};
    nearr.x = viewProj.mat[0][2];
    nearr.y = viewProj.mat[1][2];
    nearr.z = viewProj.mat[2][2];
    nearr.w = viewProj.mat[3][2];
    frustumPlanes.push_back(nearr);

    Vec4f farr{};
    farr.x = viewProj.mat[0][3] - viewProj.mat[0][2];
    farr.y = viewProj.mat[1][3] - viewProj.mat[1][2];
    farr.z = viewProj.mat[2][3] - viewProj.mat[2][2];
    farr.w = viewProj.mat[3][3] - viewProj.mat[3][2];
    frustumPlanes.push_back(farr);

    for (int i = 0; i < frustumPlanes.size(); i++) {
        Vec4f& plane = frustumPlanes[i];
        float length = sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
        plane.x /= length;
        plane.y /= length;
        plane.z /= length;
        plane.w /= length;
    }

    return frustumPlanes;
}
