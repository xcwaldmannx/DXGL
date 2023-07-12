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
		rotation.mat[0][1] = cos(rot.y) * sin(rot.z);
		rotation.mat[0][2] = -sin(rot.y);
		rotation.mat[1][0] = sin(rot.x) * sin(rot.y) * cos(rot.z) - cos(rot.x) * sin(rot.z);
		rotation.mat[1][1] = sin(rot.x) * sin(rot.y) * sin(rot.z) + cos(rot.x) * cos(rot.z);
		rotation.mat[1][2] = sin(rot.x) * cos(rot.y);
		rotation.mat[2][0] = cos(rot.x) * sin(rot.y) * cos(rot.z) + sin(rot.x) * sin(rot.z);
		rotation.mat[2][1] = cos(rot.x) * sin(rot.y) * sin(rot.z) - sin(rot.x) * cos(rot.z);
		rotation.mat[2][2] = cos(rot.x) * cos(rot.y);

		Mat4f result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.mat[i][j] =
					mat[i][0] * rotation.mat[0][j] +
					mat[i][1] * rotation.mat[1][j] +
					mat[i][2] * rotation.mat[2][j] +
					mat[i][3] * rotation.mat[3][j];
			}
		}

		memcpy(mat, result.mat, sizeof(float) * 16);
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

		v1 = Vec4f(this->mat[0][0], this->mat[1][0], this->mat[2][0], this->mat[3][0]);
		v2 = Vec4f(this->mat[0][1], this->mat[1][1], this->mat[2][1], this->mat[3][1]);
		v3 = Vec4f(this->mat[0][2], this->mat[1][2], this->mat[2][2], this->mat[3][2]);


		minor.cross(v1, v2, v3);
		det = -(this->mat[0][3] * minor.x + this->mat[1][3] * minor.y + this->mat[2][3] * minor.z + this->mat[3][3] * minor.w);
		return det;
	}

	void inverse() {
		int a, i, j;
		Mat4f out;
		Vec4f v, vec[3];
		float det = 0.0f;

		det = this->getDeterminant();
		if (!det) return;
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 4; j++) {
				if (j != i) {
					a = j;
					if (j > i) a = a - 1;
					vec[a].x = (this->mat[j][0]);
					vec[a].y = (this->mat[j][1]);
					vec[a].z = (this->mat[j][2]);
					vec[a].w = (this->mat[j][3]);
				}
			}
			v.cross(vec[0], vec[1], vec[2]);

			out.mat[0][i] = (float)pow(-1.0f, i) * v.x / det;
			out.mat[1][i] = (float)pow(-1.0f, i) * v.y / det;
			out.mat[2][i] = (float)pow(-1.0f, i) * v.z / det;
			out.mat[3][i] = (float)pow(-1.0f, i) * v.w / det;
		}

		this->setMatrix(out);
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