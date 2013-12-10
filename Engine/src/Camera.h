#pragma once

class Camera
{
public:
	Camera(PVideo video);

	f32 getAspectRatio() const { return aspectRatio; }
	void setAspectRatio(f32 aspectRatio) { this->aspectRatio = aspectRatio; updateProjectionMatrix(); }

	f32 getFarValue() const { return farValue; }
	void setFarValue(f32 farValue) { this->farValue = farValue; updateProjectionMatrix(); }

	f32 getNearValue() const { return nearValue; }
	void setNearValue(f32 nearValue) { this->nearValue = nearValue; updateProjectionMatrix(); }

	f32 getFOV() const { return fovy; }
	void setFOV(f32 fovy) { this->fovy = fovy; updateProjectionMatrix(); }

	const Vector3D &getPosition() const { return position; }
	void setPosition(const Vector3D &position)
	{
		this->lookAt += position - this->position;
		this->position = position;
		updateViewMatrix();
	}

	const Vector3D &getLookAt() const { return lookAt; }
	void setLookAt(const Vector3D &lookAt) { this->lookAt = lookAt; updateViewMatrix(); }
	const Vector3D &getUpVector() const { return upVector; }
	void setUpVector(const Vector3D &upVector) { this->upVector = upVector; updateViewMatrix(); }

	const Matrix4 &getViewMatrix() const { return viewMatrix; }
	const Matrix4 &getProjectionMatrix() const { return projectionMatrix; }

private:
	void updateViewMatrix();
	void updateProjectionMatrix();

	PVideo video;
	f32 aspectRatio, farValue, nearValue, fovy;
	Vector3D position, lookAt, upVector;
	Matrix4 viewMatrix, projectionMatrix;
};
