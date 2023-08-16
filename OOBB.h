#pragma once

#include <vector>
#include <cmath>  // Include the cmath header for std::abs

#include "Vec3f.h"
#include "Mat4f.h"

class OOBB {
public:
    OOBB(const Vec3f& center, const Vec3f& extents, const Mat4f& matrix) : m_center(center), m_extents(extents) {
        transform(matrix);
    }

    ~OOBB() {
    }

    bool isCollided(const OOBB& oobb) {
        std::vector<Vec3f> axes;
        axes.insert(axes.end(), m_axes.begin(), m_axes.end());
        axes.insert(axes.end(), oobb.m_axes.begin(), oobb.m_axes.end());

        // Generate additional separating axes using cross products
        for (const Vec3f& axis1 : m_axes) {
            for (const Vec3f& axis2 : oobb.m_axes) {
                axes.push_back(Vec3f::normalize(Vec3f::cross(axis1, axis2)));
            }
        }

        for (const auto& axis : axes) {
            Vec3f projectionA = abs(Vec3f::dot(oobb.m_center - m_center, axis));
            Vec3f projectionB =
                abs(Vec3f::dot(m_axes[0] * m_extents.x, axis)) +
                abs(Vec3f::dot(m_axes[1] * m_extents.y, axis)) +
                abs(Vec3f::dot(m_axes[2] * m_extents.z, axis)) +
                abs(Vec3f::dot(oobb.m_axes[0] * oobb.m_extents.x, axis)) +
                abs(Vec3f::dot(oobb.m_axes[1] * oobb.m_extents.y, axis)) +
                abs(Vec3f::dot(oobb.m_axes[2] * oobb.m_extents.z, axis));

            if (Vec3f::length(projectionA) > Vec3f::length(projectionB)) {
                return false;
            }
        }

        return true; // No separation along any axis, collision detected
    }

    void transform(const Mat4f& matrix) {
        // Transform the center using the transformation matrix
        m_center = matrix * m_center; // Apply the transformation

        // Transform the extents: Apply scaling factors
        m_extents.x = Vec3f::length(Vec3f(matrix.mat[0][0], matrix.mat[1][0], matrix.mat[2][0])) * m_extents.x;
        m_extents.y = Vec3f::length(Vec3f(matrix.mat[0][1], matrix.mat[1][1], matrix.mat[2][1])) * m_extents.y;
        m_extents.z = Vec3f::length(Vec3f(matrix.mat[0][2], matrix.mat[1][2], matrix.mat[2][2])) * m_extents.z;

        // Transform the axes: Apply the rotation part of the transformation matrix to each axis
        m_axes[0] = Vec3f::normalize(Vec3f(matrix.mat[0][0], matrix.mat[1][0], matrix.mat[2][0]));
        m_axes[1] = Vec3f::normalize(Vec3f(matrix.mat[0][1], matrix.mat[1][1], matrix.mat[2][1]));
        m_axes[2] = Vec3f::normalize(Vec3f(matrix.mat[0][2], matrix.mat[1][2], matrix.mat[2][2]));

        // Re-orthogonalize the axes to ensure they remain orthogonal after transformation
        m_axes[1] = Vec3f::normalize(m_axes[1] - m_axes[0] * Vec3f::dot(m_axes[1], m_axes[0]));
        m_axes[2] = Vec3f::normalize(m_axes[2] - (m_axes[0] * Vec3f::dot(m_axes[2], m_axes[0]) + m_axes[1] * Vec3f::dot(m_axes[2], m_axes[1])));
        m_axes[0] = Vec3f::cross(m_axes[1], m_axes[2]); // Regenerate the first axis using cross product
    }

public:
    Vec3f m_center;       // Center of the OOBB
    Vec3f m_extents;      // Half-lengths along the OOBB's local axes
    std::array<Vec3f, 3> m_axes;  // Local axes of the OOBB (in world space)

    // Implement methods for transforming, calculating axes, and projecting

};