#include <Core.h>

#include "Camera.h"
#include "Shader.h"
#include "Video.h"

Camera::Camera(PVideo _video):
	video(_video),
	position(Vector3D(0, 0, 0)),
	lookAt(Vector3D(0, 0, -100)),
	upVector(Vector3D(0, 1, 0)),
	nearValue(-1.0f),
	farValue(-3000.0f)
{
	fovy = 3.1415926f / 2.5f; /* Field of view in radians */
	aspectRatio = 16.0f / 9.0f; /* TODO */

	updateViewMatrix();
	updateProjectionMatrix();
}

void Camera::updateViewMatrix()
{
	viewMatrix = Matrix4::lookAtRH(position, lookAt, upVector);
}

void Camera::updateProjectionMatrix()
{
	projectionMatrix = Matrix4::perspectiveFovRH(fovy, aspectRatio, nearValue, farValue);
}

void Camera::apply(const Matrix4 &modelMatrix)
{
	PVertexShader vertexShader = video->getVertexShader();
	if (!vertexShader)
		return;
	vertexShader->setShaderConstant("mvpMatrix", modelMatrix * viewMatrix * projectionMatrix);
}
