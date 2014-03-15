#pragma once

class Shader
{
public:
	virtual ~Shader() {}

	virtual void setShaderConstant(const std::string &name, const Matrix4 &matrix) = 0;
};

class VertexShader: public Shader
{
public:
	virtual ~VertexShader() {}
};

class PixelShader: public Shader
{
public:
	virtual ~PixelShader() {}
};

class GeometryShader: public Shader
{
public:
	virtual ~GeometryShader() {}
};

class HullShader: public Shader
{
public:
	virtual ~HullShader() {}
};

class DomainShader: public Shader
{
public:
	virtual ~DomainShader() {}
};

class ComputeShader: public Shader
{
public:
	virtual ~ComputeShader() {}
};
