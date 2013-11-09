#pragma once

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

	const vector3df &getPosition() const { return position; }
	void setPosition(const vector3df &position)
	{
		this->lookAt += position - this->position;
		this->position = position;
		updateViewMatrix();
	}

	const vector3df &getLookAt() const { return lookAt; }
	void setLookAt(const vector3df &lookAt) { this->lookAt = lookAt; updateViewMatrix(); }
	const vector3df &getUpVector() const { return upVector; }
	void setUpVector(const vector3df &upVector) { this->upVector = upVector; updateViewMatrix(); }

	const matrix4 &getViewMatrix() const { return viewMatrix; }
	const matrix4 &getProjectionMatrix() const { return projectionMatrix; }

private:
	void updateViewMatrix();
	void updateProjectionMatrix();

	IVideo *video;
	f32 aspectRatio, farValue, nearValue, fovy;
	vector3df position, lookAt, upVector;
	matrix4 viewMatrix, projectionMatrix;
};
