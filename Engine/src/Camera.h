#pragma once

class Camera
{
public:
	Camera(PVideo video);

	float getAspectRatio() const { return aspectRatio; }
	void setAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio; updateProjectionMatrix(); }

	float getFarValue() const { return farValue; }
	void setFarValue(float farValue) { this->farValue = farValue; updateProjectionMatrix(); }

	float getNearValue() const { return nearValue; }
	void setNearValue(float nearValue) { this->nearValue = nearValue; updateProjectionMatrix(); }

	float getFOV() const { return fovy; }
	void setFOV(float fovy) { this->fovy = fovy; updateProjectionMatrix(); }

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

	void apply(const Matrix4 &modelMatrix);

private:
	void updateViewMatrix();
	void updateProjectionMatrix();

	PVideo video;
	float aspectRatio, farValue, nearValue, fovy;
	Vector3D position, lookAt, upVector;
	Matrix4 viewMatrix, projectionMatrix;
};
