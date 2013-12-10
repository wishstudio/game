#include <Core.h>

#include "GUI.h"
#include "Material.h"
#include "Pass.h"
#include "VertexBuffer.h"
#include "VertexFormat.h"
#include "Video.h"

static const char SHADER2D_SRC[] = R"DELIM(
cbuffer MatrixBuffer /* TODO: This is kept to be compatible with normal shaders */
{
	float4x4 mvpMatrix;
};

struct VS_InputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD;
};

struct PS_InputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD;
};

PS_InputType VS_Main(VS_InputType input)
{
	return input;
}

Texture2D shaderTexture: register(t0);
SamplerState shaderSampler: register(s0);

float4 PS_Main(PS_InputType input): SV_TARGET
{
	return shaderTexture.Sample(shaderSampler, input.tex) * input.color;
}

)DELIM";

struct Vertex2D
{
	Vector2D position;
	Color color;
	Vector2D tex;
};

GUI::GUI(PVideo _video):
	video(_video)
{
	Color whiteData(255, 255, 255, 255);
	whiteTexture = video->createTexture(1, 1, &whiteData);
	material = video->createMaterial();
	PPass pass = material->createPass();
	pass->setVertexShader(video->createVertexShader(SHADER2D_SRC, "VS_Main"));
	pass->setPixelShader(video->createPixelShader(SHADER2D_SRC, "PS_Main"));

	PVertexFormat vertexFormat = video->createVertexFormat();
	vertexFormat->addElement(TYPE_FLOAT2, SEMANTIC_POSITION);
	vertexFormat->addElement(TYPE_UBYTE4_NORM, SEMANTIC_COLOR);
	vertexFormat->addElement(TYPE_FLOAT2, SEMANTIC_TEXCOORD);

	/* Create a large enough vertex buffer */
	vertexBuffer = video->createVertexBuffer(vertexFormat, 128);
}

#define NORMALIZE_POS(_x, _y) Vector2D((_x) * 2 / (f32) bbSize.x - 1.f, 1.f - (_y) * 2 / (f32) bbSize.y)
void GUI::draw2DLine(const Vector2DI &start, const Vector2DI &end, Color color)
{
	Vector2DI bbSize = video->getBackBufferSize();

	Vertex2D vertices[2] = {
		{ NORMALIZE_POS(start.x, start.y), color, Vector2D(0, 0) },
		{ NORMALIZE_POS(end.x, end.y), color, Vector2D(0, 0) }
	};
	vertexBuffer->update(0, 2, vertices);

	video->setMaterial(material);
	video->setTexture(whiteTexture);
	video->draw(vertexBuffer, 0, 2, TOPOLOGY_LINELIST);
}

void GUI::draw2DRect(const Vector2DI &topLeft, const Vector2DI &bottomRight, Color color)
{
	Vector2DI bbSize = video->getBackBufferSize();

	Vertex2D vertices[5] = {
		{ NORMALIZE_POS(topLeft.x, topLeft.y), color, Vector2D(0, 0) },
		{ NORMALIZE_POS(bottomRight.x, topLeft.y), color, Vector2D(0, 0) },
		{ NORMALIZE_POS(bottomRight.x, bottomRight.y), color, Vector2D(0, 0) },
		{ NORMALIZE_POS(topLeft.x, bottomRight.y), color, Vector2D(0, 0) },
		{ NORMALIZE_POS(topLeft.x, topLeft.y), color, Vector2D(0, 0) }
	};
	vertexBuffer->update(0, 5, vertices);

	video->setMaterial(material);
	video->setTexture(whiteTexture);
	video->draw(vertexBuffer, 0, 5, TOPOLOGY_LINESTRIP);
}

void GUI::draw2DTexture(const Vector2DI &pos, const Vector2DI &size, PTexture texture)
{
	Vector2DI bbSize = video->getBackBufferSize();

	Vertex2D vertices[6] = {
		{ NORMALIZE_POS(pos.x, pos.y), Color(255, 255, 255, 255), Vector2D(0, 0) },
		{ NORMALIZE_POS(pos.x, pos.y + size.y), Color(255, 255, 255, 255), Vector2D(0, 1) },
		{ NORMALIZE_POS(pos.x + size.x, pos.y), Color(255, 255, 255, 255), Vector2D(1, 0) },

		{ NORMALIZE_POS(pos.x + size.x, pos.y), Color(255, 255, 255, 255), Vector2D(1, 0) },
		{ NORMALIZE_POS(pos.x, pos.y + size.y), Color(255, 255, 255, 255), Vector2D(0, 1) },
		{ NORMALIZE_POS(pos.x + size.x, pos.y + size.y), Color(255, 255, 255, 255), Vector2D(1, 1) }
	};
	vertexBuffer->update(0, 6, vertices);

	video->setMaterial(material);
	video->setTexture(texture);
	video->draw(vertexBuffer, 0, 6, TOPOLOGY_TRIANGLELIST);
}

void GUI::fill2DRect(const Vector2DI &topLeft, const Vector2DI &bottomRight, Color color)
{
	Vector2DI bbSize = video->getBackBufferSize();

	Vertex2D vertices[6] = {
		{ NORMALIZE_POS(topLeft.x, topLeft.y), color, Vector2D(0, 0) },
		{ NORMALIZE_POS(topLeft.x, bottomRight.y), color, Vector2D(0, 0) },
		{ NORMALIZE_POS(bottomRight.x, topLeft.y), color, Vector2D(0, 0) },

		{ NORMALIZE_POS(bottomRight.x, topLeft.y), color, Vector2D(0, 0) },
		{ NORMALIZE_POS(topLeft.x, bottomRight.y), color, Vector2D(0, 0) },
		{ NORMALIZE_POS(bottomRight.x, bottomRight.y), color, Vector2D(0, 0) },
	};
	vertexBuffer->update(0, 6, vertices);

	video->setMaterial(material);
	video->setTexture(whiteTexture);
	video->draw(vertexBuffer, 0, 6, TOPOLOGY_TRIANGLELIST);
}
