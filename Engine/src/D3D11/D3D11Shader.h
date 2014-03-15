#pragma once

#include "../Shader.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
class D3D11Backend;

struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11GeometryShader;
struct ID3D11HullShader;
struct ID3D11DomainShader;
struct ID3D11ComputeShader;

class D3D11Shader
{
public:
	D3D11Shader(ID3D11Device *device, D3D11Backend *shaderReflection);
	virtual ~D3D11Shader();

	void setShaderConstant(const std::string &name, const Matrix4 &matrix);
	void syncConstantBuffers(ID3D11DeviceContext *context);
	int getConstantBufferCount() const { return constantBuffers.size(); }
	ID3D11Buffer * const *getConstantBuffers() const { return constantBuffers.data(); }

private:
	class ConstantBufferCache
	{
	public:
		ConstantBufferCache() = default;
		ConstantBufferCache(ConstantBufferCache &&another) /* TODO: Use =default when VC supports it */
			: data(std::move(another.data)), dirty(another.dirty) {}

		std::unique_ptr<char> data;
		bool dirty;
	};
	std::unique_ptr<D3D11Backend> shaderReflection;
	std::vector<ID3D11Buffer *> constantBuffers;
	std::vector<ConstantBufferCache> constantBufferCaches;
	std::unordered_map<std::string, D3D11ConstantReflection *> constantsMap;
};

#define IMPLEMENT_SHADER_BASE_FUNCTIONS() \
	virtual void setShaderConstant(const std::string &name, const Matrix4 &matrix) override { D3D11Shader::setShaderConstant(name, matrix); }

class D3D11VertexShader: public D3D11Shader, public VertexShader
{
public:
	D3D11VertexShader(ID3D11Device *device, ID3D11VertexShader *_pVertexShader, D3D11Backend *shaderReflection, ID3DBlob *_pBytecode)
		: D3D11Shader(device, shaderReflection), pVertexShader(_pVertexShader), pBytecode(_pBytecode) {}
	virtual ~D3D11VertexShader() override;

	ID3D11VertexShader *getVertexShader() const { return pVertexShader; }
	ID3DBlob *getBytecode() const { return pBytecode; }

	IMPLEMENT_SHADER_BASE_FUNCTIONS();

private:
	ID3D11VertexShader *pVertexShader;
	ID3DBlob *pBytecode;
};

class D3D11PixelShader: public D3D11Shader, public PixelShader
{
public:
	D3D11PixelShader(ID3D11Device *device, ID3D11PixelShader *_pPixelShader, D3D11Backend *shaderReflection)
		: D3D11Shader(device, shaderReflection), pPixelShader(_pPixelShader) {}
	virtual ~D3D11PixelShader() override;

	ID3D11PixelShader *getPixelShader() const { return pPixelShader; }

	IMPLEMENT_SHADER_BASE_FUNCTIONS();

private:
	ID3D11PixelShader *pPixelShader;
};

class D3D11GeometryShader: public D3D11Shader, public GeometryShader
{
public:
	D3D11GeometryShader(ID3D11Device *device, ID3D11GeometryShader *_pGeometryShader, D3D11Backend *shaderReflection)
		: D3D11Shader(device, shaderReflection), pGeometryShader(_pGeometryShader) {}
	virtual ~D3D11GeometryShader() override;

	ID3D11GeometryShader *getGeometryShader() const { return pGeometryShader; }

	IMPLEMENT_SHADER_BASE_FUNCTIONS();

private:
	ID3D11GeometryShader *pGeometryShader;
};

class D3D11HullShader: public D3D11Shader, public HullShader
{
public:
	D3D11HullShader(ID3D11Device *device, ID3D11HullShader *_pHullShader, D3D11Backend *shaderReflection)
		: D3D11Shader(device, shaderReflection), pHullShader(_pHullShader) {}
	virtual ~D3D11HullShader() override;

	ID3D11HullShader *getHullShader() const { return pHullShader; }

	IMPLEMENT_SHADER_BASE_FUNCTIONS();

private:
	ID3D11HullShader *pHullShader;
};

class D3D11DomainShader: public D3D11Shader, public DomainShader
{
public:
	D3D11DomainShader(ID3D11Device *device, ID3D11DomainShader *_pDomainShader, D3D11Backend *shaderReflection)
		: D3D11Shader(device, shaderReflection), pDomainShader(_pDomainShader) {}
	virtual ~D3D11DomainShader() override;

	ID3D11DomainShader *getDomainShader() const { return pDomainShader; }

	IMPLEMENT_SHADER_BASE_FUNCTIONS();

private:
	ID3D11DomainShader *pDomainShader;
};

class D3D11ComputeShader: public D3D11Shader, public ComputeShader
{
public:
	D3D11ComputeShader(ID3D11Device *device, ID3D11ComputeShader *_pComputeShader, D3D11Backend *shaderReflection)
		: D3D11Shader(device, shaderReflection), pComputeShader(_pComputeShader) {}
	virtual ~D3D11ComputeShader() override;

	ID3D11ComputeShader *getComputeShader() const { return pComputeShader; }

	IMPLEMENT_SHADER_BASE_FUNCTIONS();

private:
	ID3D11ComputeShader *pComputeShader;
};

#undef IMPLEMENT_SHADER_BASE_FUNCTIONS
