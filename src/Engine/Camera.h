#pragma once

#include "Vector3.h"

class IVideo;
class Camera
{
public:
	Camera(IVideo *video);

	f32 getAspectRatio() const { return aspectRatio; }
	void setAspectRatio(f32 aspectRatio) { this->aspectRatio = aspectRatio; updateProjectionMatrix(); }

	f32 getFarValue() const { return farValue; }
	void setFarValue(f32 farValue) { this->farValue = farValue; updateProjectionMatrix(); }

	f32 getNearValue() const { return nearValue; }
	void setNearValue(f32 nearValue) { this->nearValue = nearValue; updateProjectionMatrix(); }

	f32 getFOV() const { return fovy; }
	void setFOV(f32 fovy) { this->fovy = fovy; updateProjectionMatrix(); }

	const Vector3 &getPosition() const { return position; }
	void setPosition(const Vector3 &position)
	{
		this->lookAt += position - this->position;
		this->position = position;
		updateViewMatrix();
	}

	const Vector3 &getLookAt() const { return lookAt; }
	void setLookAt(const Vector3 &lookAt) { this->lookAt = lookAt; updateViewMatrix(); }
	const Vector3 &getUpVector() const { return upVector; }
	void setUpVector(const Vector3 &upVector) { this->upVector = upVector; updateViewMatrix(); }

	const Matrix4 &getViewMatrix() const { return viewMatrix; }
	const Matrix4 &getProjectionMatrix() const { return projectionMatrix; }

private:
	void updateViewMatrix();
	void updateProjectionMatrix();

	IVideo *video;
	f32 aspectRatio, farValue, nearValue, fovy;
	Vector3 position, lookAt, upVector;
	Matrix4 viewMatrix, projectionMatrix;
};
