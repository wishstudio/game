#pragma once

#include "../Shader.h"

struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11GeometryShader;
struct ID3D11HullShader;
struct ID3D11DomainShader;
struct ID3D11ComputeShader;

class D3D11VertexShader: public VertexShader
{
public:
	D3D11VertexShader(ID3D11VertexShader *_pVertexShader, ID3DBlob *_pBytecode):
		pVertexShader(_pVertexShader), pBytecode(_pBytecode) {}
	virtual ~D3D11VertexShader();

	ID3D11VertexShader *getVertexShader() const { return pVertexShader; }
	ID3DBlob *getBytecode() const { return pBytecode; }

private:
	ID3D11VertexShader *pVertexShader;
	ID3DBlob *pBytecode;
};

class D3D11PixelShader: public PixelShader
{
public:
	D3D11PixelShader(ID3D11PixelShader *_pPixelShader): pPixelShader(_pPixelShader) {}
	virtual ~D3D11PixelShader();

	ID3D11PixelShader *getPixelShader() const { return pPixelShader; }

private:
	ID3D11PixelShader *pPixelShader;
};

class D3D11GeometryShader: public GeometryShader
{
public:
	D3D11GeometryShader(ID3D11GeometryShader *_pGeometryShader): pGeometryShader(_pGeometryShader) {}
	virtual ~D3D11GeometryShader();

	ID3D11GeometryShader *getGeometryShader() const { return pGeometryShader; }

private:
	ID3D11GeometryShader *pGeometryShader;
};

class D3D11HullShader: public HullShader
{
public:
	D3D11HullShader(ID3D11HullShader *_pHullShader): pHullShader(_pHullShader) {}
	virtual ~D3D11HullShader();

	ID3D11HullShader *getHullShader() const { return pHullShader; }

private:
	ID3D11HullShader *pHullShader;
};

class D3D11DomainShader: public DomainShader
{
public:
	D3D11DomainShader(ID3D11DomainShader *_pDomainShader): pDomainShader(_pDomainShader) {}
	virtual ~D3D11DomainShader();

	ID3D11DomainShader *getDomainShader() const { return pDomainShader; }

private:
	ID3D11DomainShader *pDomainShader;
};

class D3D11ComputeShader: public ComputeShader
{
public:
	D3D11ComputeShader(ID3D11ComputeShader *_pComputeShader): pComputeShader(_pComputeShader) {}
	virtual ~D3D11ComputeShader();

	ID3D11ComputeShader *getComputeShader() const { return pComputeShader; }

private:
	ID3D11ComputeShader *pComputeShader;
};
