#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "IRBase.h"

class IRType: public IRNode
{
public:
	enum TypeKind
	{
		_VALUE_TYPE_BEGIN,
		Primitive,
		Vector,
		Matrix,
		_VALUE_TYPE_END,

		Struct,

		_SHADER_OBJECT_BEGIN,
		SamplerState,
		Texture2D,
		_SHADER_OBJECT_END,
	};
	enum PrimitiveKind { Float };
	IRType(TypeKind _kind): IRNode(IRNode::Type), kind(_kind) {}
	virtual ~IRType() {}

	bool getIsValueType() const { return kind > _VALUE_TYPE_BEGIN && kind < _VALUE_TYPE_END; }
	bool getIsPrimitive() const { return kind == Primitive; }
	bool getIsVector() const { return kind == Vector; }
	bool getIsMatrix() const { return kind == Matrix; }
	bool getIsStruct() const { return kind == Struct; }
	bool getIsShaderObject() const { return kind > _SHADER_OBJECT_BEGIN && kind < _SHADER_OBJECT_END; }
	bool getIsSamplerState() const { return kind == SamplerState; }
	bool getIsTexture2D() const { return kind == Texture2D; }

private:
	TypeKind kind;
};

class IRValueType: public IRType
{
public:
	IRValueType(TypeKind kind): IRType(kind) {}
	virtual ~IRValueType() {}

	virtual int getSize() = 0;
};

class IRPrimitiveType: public IRValueType
{
public:
	IRPrimitiveType(PrimitiveKind _kind): IRValueType(IRType::Primitive), kind(_kind) {}
	virtual ~IRPrimitiveType() override {}

	PrimitiveKind getPrimitiveKind() const { return kind; }

	virtual int getSize() override
	{
		if (kind == IRType::Float)
			return 4;
		return 0; /* Make compiler happy */
	}

private:
	PrimitiveKind kind;
};

class IRVectorType: public IRValueType
{
public:
	IRVectorType(IRPrimitiveType *_base, int _dim): IRValueType(IRType::Vector), base(_base), dim(_dim) {}
	virtual ~IRVectorType() override {}

	IRPrimitiveType *getBaseType() const { return base; }
	int getDimension() const { return dim; }

	virtual int getSize() override
	{
		return base->getSize() * dim;
	}

private:
	IRPrimitiveType *base;
	int dim;
};

class IRMatrixType: public IRValueType
{
public:
	IRMatrixType(IRPrimitiveType *_base, int _rowdim, int _coldim): IRValueType(IRType::Matrix), base(_base), rowdim(_rowdim), coldim(_coldim) {}
	virtual ~IRMatrixType() override {}

	IRPrimitiveType *getBaseType() const { return base; }
	int getRowDimension() const { return rowdim; }
	int getColumnDimension() const { return coldim; }

	virtual int getSize() override
	{
		return base->getSize() * rowdim * coldim;
	}

private:
	IRPrimitiveType *base;
	int rowdim, coldim;
};

class IRStructType: public IRType
{
public:
	IRStructType(const std::string &_name): IRType(IRType::Struct), name(_name) {}
	virtual ~IRStructType() override {}

	std::string getName() const { return name; }

	void addField(IRType *type, const std::string &name, const std::string &semantic = std::string())
	{
		fields.push_back({ type, name, semantic });
	}
	int getFieldCount() const { return fields.size(); }
	IRType *getFieldType(int index) const { return fields[index].type; }
	std::string getFieldName(int index) const { return fields[index].name; }
	std::string getFieldSemantic(int index) const { return fields[index].semantic; }

private:
	struct Field
	{
		IRType *type;
		std::string name, semantic;
	};
	std::string name;
	std::vector<Field> fields;
};

class IRFunction;
class IRShaderObject: public IRType
{
public:
	IRShaderObject(IRType::TypeKind kind): IRType(kind) {}
	virtual ~IRShaderObject() {}

	virtual IRFunction *getFunction(const std::string &name) { return nullptr; }
};

class IRSamplerState: public IRShaderObject
{
public:
	IRSamplerState(): IRShaderObject(IRType::SamplerState) {}
	virtual ~IRSamplerState() {}
};

class IRTexture2D: public IRShaderObject
{
public:
	IRTexture2D(): IRShaderObject(IRType::Texture2D) {}
	virtual ~IRTexture2D() {}

	virtual IRFunction *getFunction(const std::string &name) override;
};
