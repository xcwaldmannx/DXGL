#pragma once

#include <memory>

#include "Vec3f.h"
#include "Vec4f.h"

class Mat4f {
public:
	Mat4f() {
	}

	Mat4f(float matrix[4][4]) {
		memcpy(mat, matrix, sizeof(float) * 16);
	}

	~Mat4f() {
	}

	void setIdentity() {
		memset(mat, 0, sizeof(float) * 16);
		mat[0][0] = 1;
		mat[1][1] = 1;
		mat[2][2] = 1;
		mat[3][3] = 1;
	}

	void setTranslation(Vec3f trans) {
		mat[3][0] = trans.x;
		mat[3][1] = trans.y;
		mat[3][2] = trans.z;
	}

	void setScale(Vec3f scale) {
		mat[0][0] = scale.x;
		mat[1][1] = scale.y;
		mat[2][2] = scale.z;
	}

	void setRotationX(float x) {
		mat[1][1] = cos(x);
		mat[1][2] = sin(x);
		mat[2][1] = -sin(x);
		mat[2][2] = cos(x);
	}

	void setRotationY(float y) {
		mat[0][0] = cos(y);
		mat[0][2] = -sin(y);
		mat[2][0] = sin(y);
		mat[2][2] = cos(y);
	}

	void setRotationZ(float z) {
		mat[0][0] = cos(z);
		mat[0][1] = sin(z);
		mat[1][0] = -sin(z);
		mat[1][1] = cos(z);
	}

	void setRotation(const Vec3f& rot) {
		Mat4f rotation;
		rotation.setIdentity();

		rotation.mat[0][0] = cos(rot.y) * cos(rot.z);
		rotation.mat[0][1] = -cos(rot.y) * sin(rot.z);
		rotation.mat[0][2] = sin(rot.y);
		rotation.mat[1][0] = sin(rot.x) * sin(rot.y) * cos(rot.z) + cos(rot.x) * sin(rot.z);
		rotation.mat[1][1] = -sin(rot.x) * sin(rot.y) * sin(rot.z) + cos(rot.x) * cos(rot.z);
		rotation.mat[1][2] = -sin(rot.x) * cos(rot.y);
		rotation.mat[2][0] = cos(rot.x) * sin(rot.y) * cos(rot.z) - sin(rot.x) * sin(rot.z);
		rotation.mat[2][1] = cos(rot.x) * sin(rot.y) * sin(rot.z) + sin(rot.x) * cos(rot.z);
		rotation.mat[2][2] = cos(rot.x) * cos(rot.y);

		// Multiply the rotation matrix with the existing transformation matrix
		Mat4f result = rotation * (*this);

		// Copy the result back to the current transformation matrix
		memcpy(mat, result.mat, sizeof(float) * 16);
	}

	void setTransform(const Vec3f& scale, const Vec3f& rotation, const Vec3f& translation) {
		Mat4f transform;

		// Set the scale
		transform.mat[0][0] = scale.x;
		transform.mat[1][1] = scale.y;
		transform.mat[2][2] = scale.z;
		transform.mat[3][3] = 1.0f;

		Mat4f rotationMatrix;
		rotationMatrix.setIdentity();

		// set the rotations
		float cx = cos(rotation.x);
		float sx = sin(rotation.x);
		float cy = cos(rotation.y);
		float sy = sin(rotation.y);
		float cz = cos(rotation.z);
		float sz = sin(rotation.z);

		// Calculate individual rotation matrices for each axis
		Mat4f xRotation;
		xRotation.setIdentity();
		xRotation.mat[1][1] = cx;
		xRotation.mat[1][2] = -sx;
		xRotation.mat[2][1] = sx;
		xRotation.mat[2][2] = cx;

		Mat4f yRotation;
		yRotation.setIdentity();
		yRotation.mat[0][0] = cy;
		yRotation.mat[0][2] = sy;
		yRotation.mat[2][0] = -sy;
		yRotation.mat[2][2] = cy;

		Mat4f zRotation;
		zRotation.setIdentity();
		zRotation.mat[0][0] = cz;
		zRotation.mat[0][1] = -sz;
		zRotation.mat[1][0] = sz;
		zRotation.mat[1][1] = cz;

		rotationMatrix = zRotation * (yRotation * xRotation);

		// Multiply rotation with scale
		transform = rotationMatrix * transform;

		// Set the translation
		transform.mat[3][0] = translation.x;
		transform.mat[3][1] = translation.y;
		transform.mat[3][2] = translation.z;
		transform.mat[3][3] = 1.0f;

		// Copy the result back to the current transformation matrix
		memcpy(mat, transform.mat, sizeof(float) * 16);
	}

	void lookAt(Vec3f position, Vec3f target, Vec3f up) {

		Vec3f zaxis = Vec3f::normalize(target - position);
		Vec3f xaxis = Vec3f::normalize(Vec3f::cross(up, zaxis));
		Vec3f yaxis = Vec3f::cross(zaxis, xaxis);

		mat[0][0] = xaxis.x;
		mat[0][1] = yaxis.x;
		mat[0][2] = zaxis.x;
		mat[0][3] = 0.0f;

		mat[1][0] = xaxis.y;
		mat[1][1] = yaxis.y;
		mat[1][2] = zaxis.y;
		mat[1][3] = 0.0f;

		mat[2][0] = xaxis.z;
		mat[2][1] = yaxis.z;
		mat[2][2] = zaxis.z;
		mat[2][3] = 0.0f;

		mat[3][0] = -Vec3f::dot(xaxis, position);
		mat[3][1] = -Vec3f::dot(yaxis, position);
		mat[3][2] = -Vec3f::dot(zaxis, position);
		mat[3][3] = 1.0f;
	}

	float getDeterminant() {
		Vec4f minor, v1, v2, v3;
		float det;

		v1 = Vec4f(mat[0][0], mat[1][0], mat[2][0], mat[3][0]);
		v2 = Vec4f(mat[0][1], mat[1][1], mat[2][1], mat[3][1]);
		v3 = Vec4f(mat[0][2], mat[1][2], mat[2][2], mat[3][2]);


		minor.cross(v1, v2, v3);
		det = -(mat[0][3] * minor.x + mat[1][3] * minor.y + mat[2][3] * minor.z + mat[3][3] * minor.w);
		return det;
	}

	void inverse() {
		int a, i, j;
		Mat4f out;
		Vec4f v, vec[3];
		float det = 0.0f;

		det = getDeterminant();
		if (!det) return;
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 4; j++) {
				if (j != i) {
					a = j;
					if (j > i) a = a - 1;
					vec[a].x = (mat[j][0]);
					vec[a].y = (mat[j][1]);
					vec[a].z = (mat[j][2]);
					vec[a].w = (mat[j][3]);
				}
			}
			v.cross(vec[0], vec[1], vec[2]);

			out.mat[0][i] = (float)pow(-1.0f, i) * v.x / det;
			out.mat[1][i] = (float)pow(-1.0f, i) * v.y / det;
			out.mat[2][i] = (float)pow(-1.0f, i) * v.z / det;
			out.mat[3][i] = (float)pow(-1.0f, i) * v.w / det;
		}

		setMatrix(out);
	}

	void operator *=(const Mat4f& matrix) {
		Mat4f result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.mat[i][j] =
					mat[i][0] * matrix.mat[0][j] +
					mat[i][1] * matrix.mat[1][j] +
					mat[i][2] * matrix.mat[2][j] +
					mat[i][3] * matrix.mat[3][j];
			}
		}

		memcpy(mat, result.mat, sizeof(float) * 16);
	}

	Mat4f operator *(const Mat4f& matrix) {
		Mat4f result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.mat[i][j] =
					mat[i][0] * matrix.mat[0][j] +
					mat[i][1] * matrix.mat[1][j] +
					mat[i][2] * matrix.mat[2][j] +
					mat[i][3] * matrix.mat[3][j];
			}
		}

		return result;
	}

	Vec3f operator *(const Vec3f& vector) const {
		float x = mat[0][0] * vector.x + mat[1][0] * vector.y + mat[2][0] * vector.z + mat[3][0];
		float y = mat[0][1] * vector.x + mat[1][1] * vector.y + mat[2][1] * vector.z + mat[3][1];
		float z = mat[0][2] * vector.x + mat[1][2] * vector.y + mat[2][2] * vector.z + mat[3][2];
		float w = mat[0][3] * vector.x + mat[1][3] * vector.y + mat[2][3] * vector.z + mat[3][3];

		// Perform perspective division (homogeneous coordinate)
		if (w != 0.0f) {
			x /= w;
			y /= w;
			z /= w;
		}

		return Vec3f(x, y, z);
	}

	void setMatrix(const Mat4f& matrix) {
		memcpy(mat, matrix.mat, sizeof(float) * 16);
	}

	Vec3f getXDirection() {
		return Vec3f(mat[0][0], mat[0][1], mat[0][2]);
	}

	Vec3f getYDirection() {
		return Vec3f(mat[1][0], mat[1][1], mat[1][2]);
	}

	Vec3f getZDirection() {
		return Vec3f(mat[2][0], mat[2][1], mat[2][2]);
	}

	Vec3f getTranslation() {
		return Vec3f(mat[3][0], mat[3][1], mat[3][2]);
	}

	Vec3f getRotation() {
		Vec3f rotations{};
		rotations.x = asinf(-mat[2][1]);
		if (cosf(rotations.x) > 0.0001) {
			rotations.y = atan2f(mat[2][0], mat[2][2]);
			rotations.z = atan2f(mat[0][1], mat[1][1]);
		}
		else {
			rotations.y = 0.0f;
			rotations.z = atan2f(-mat[1][0], mat[0][0]);
		}
		return rotations;
	}

	void setOrthographic(float width, float height, float nearPlane, float farPlane) {
		setIdentity();
		mat[0][0] = 2.0f / width;
		mat[1][1] = 2.0f / height;
		mat[2][2] = 1.0f / (farPlane - nearPlane);
		mat[3][2] = -(nearPlane / (farPlane - nearPlane));

	}

	void setPerspective(float fov, float aspect, float zNear, float zFar) {
		float yScale = 1.0f / tan(fov / 2.0f);
		float xScale = yScale / aspect;
		float frustumLength = zFar - zNear;

		mat[0][0] = xScale;
		mat[1][1] = yScale;
		mat[2][2] = zFar / frustumLength;
		mat[2][3] = 1.0f;
		mat[3][2] = (-zNear * zFar) / frustumLength;
	}

	float mat[4][4] = {};
};