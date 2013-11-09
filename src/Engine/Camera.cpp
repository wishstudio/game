#include <stdafx.h>

#include "IVideo.h"

#include "Camera.h"

Camera::Camera(IVideo *_video):
	video(_video),
	position(Vector3(0, 0, 0)),
	lookAt(Vector3(0, 0, 100)),
	upVector(Vector3(0, 1, 0)),
	nearValue(1.0f),
	farValue(3000.0f)
{
	fovy = 3.1415926 / 2.5f; /* Field of view in radians */
	aspectRatio = 4.0f / 3.0f; /* TODO */

	updateViewMatrix();
	updateProjectionMatrix();
}

void Camera::updateViewMatrix()
{
	viewMatrix = Matrix4::lookAtLH(position, lookAt, upVector);
}

void Camera::updateProjectionMatrix()
{
	projectionMatrix = Matrix4::perspectiveFovLH(fovy, aspectRatio, nearValue, farValue);
}
