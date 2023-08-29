#include "Camera.h"

using namespace dxgl;

Camera::Camera() {
}

Camera::~Camera() {
}

void Camera::update(long double deltaTime) {
    // get mouse input for rotation
    Point2f delta = Engine::input()->getMouseDelta();

    m_rotX += delta.y * (float)deltaTime / 5.0f;
    m_rotY += delta.x * (float)deltaTime / 5.0f;


    // set rotation bounds
    if (m_rotX >= 1.57f) {
        m_rotX = 1.57f;
    }
    else if (m_rotX <= -1.57f) {
        m_rotX = -1.57f;
    }

    // camera rotation
    Mat4f camera, temp;
    camera.setIdentity();

    temp.setIdentity();
    temp.setRotationX(m_rotX);
    camera *= temp;
    temp.setIdentity();
    temp.setRotationY(m_rotY);
    camera *= temp;

    // set camera translation
    float forward = Engine::input()->getKeyPressState('W') - Engine::input()->getKeyPressState('S');
    float right = Engine::input()->getKeyPressState('D') - Engine::input()->getKeyPressState('A');
    float up = Engine::input()->getKeyPressState(VK_SPACE) - Engine::input()->getKeyPressState(VK_SHIFT);

    Vec3f camX{};
    Vec3f camY{};
    Vec3f camZ{};
    Vec3f currentPos = m_world.getTranslation();
    float speed = 64.0f * (float) deltaTime;

    if (Engine::input()->getMouseState(DXGLInputManager::RMB_STATE)) {
        speed = 8.0f * (float) deltaTime;
    }

    if (forward || right || up) {
        m_speed = speed;
    } else {
        m_speed = 0;
    }

    camX = m_world.getXDirection() * right * speed;
    camY = m_world.getYDirection() * up * speed;
    camZ = m_world.getZDirection() * forward * speed;
    m_position = currentPos + camX + camY + camZ;

    camera.setTranslation(m_position);

    // set world
    m_world = camera;
    camera.inverse();

    // set view
    m_view = camera;

    RECT window = Engine::window()->getWindowSize();

    int widthAspect = window.right;
    int heightAspect = window.bottom;
    float aspect = (float) widthAspect / (float) heightAspect;

    // set projection
    m_proj.setPerspective(1.2f, aspect, 0.1f, 5000.0f);

    m_direction = m_world.getZDirection();
}

Mat4f& Camera::world() {
    return m_world;
}
Mat4f& Camera::view() {
    return m_view;
}
Mat4f& Camera::proj() {
    return m_proj;
}

Vec3f Camera::getPosition() {
    return m_position;
}

Vec3f Camera::getDirection() {
    return m_direction;
}

float Camera::getCurrentSpeed() {
    return m_speed;
}

bool Camera::cull(Vec3f position, Vec3f scale, Vec3f minVertex, Vec3f maxVertex) {
    std::vector<Vec4f> planes = getFrustum();

    for (int i = 0; i < planes.size(); i++) {

        Vec4f planeNormal = Vec4f(planes[i].x, planes[i].y, planes[i].z, 0.0f);
        float planeConstant = planes[i].w;

        Vec3f axisVertex{};

        // x-axis
        if (planes[i].x < 0.0f) { // Which AABB vertex is furthest down (plane normals direction) the x axis
            axisVertex.x = minVertex.x * scale.x + position.x; // min x plus tree positions x
        } else {
            axisVertex.x = maxVertex.x * scale.x + position.x; // max x plus tree positions x
        }

        // y-axis
        if (planes[i].y < 0.0f) { // Which AABB vertex is furthest down (plane normals direction) the y axis
            axisVertex.y = minVertex.y * scale.y + position.y; // min y plus tree positions y
        } else {
            axisVertex.y = maxVertex.y * scale.y + position.y; // max y plus tree positions y
        }

        // z-axis
        if (planes[i].z < 0.0f) { // Which AABB vertex is furthest down (plane normals direction) the z axis
            axisVertex.z = minVertex.z * scale.z + position.z; // min z plus tree positions z
        } else {
            axisVertex.z = maxVertex.z * scale.z + position.z; // max z plus tree positions z
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

std::vector<Vec4f> Camera::getFrustum() {
    std::vector<Vec4f> frustumPlanes{};
    Mat4f viewProj = m_view * m_proj;

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
