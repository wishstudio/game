#pragma once

class Material;
class GUI
{
public:
	GUI(PVideo video);

	void draw2DLine(const Vector2DI &start, const Vector2DI &end, Color color);
	void draw2DRect(const Vector2DI &topLeft, const Vector2DI &bottomRight, Color color);
	void draw2DTexture(const Vector2DI &pos, const Vector2DI &size, PTexture texture);

	void fill2DRect(const Vector2DI &topLeft, const Vector2DI &bottomRight, Color color);

private:
	PVideo video;
	PTexture whiteTexture;

	Material *material = nullptr;
	PVertexShader vertexShader;
	PPixelShader pixelShader;

	PVertexBuffer vertexBuffer;
};
