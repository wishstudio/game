#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

class IRType
{
public:
	enum TypeKind { Primitive, Vector, Matrix, Struct };
	enum PrimitiveKind { Float };
	IRType(TypeKind _kind): kind(_kind) {}
	virtual ~IRType() {}

	bool getIsPrimitive() const { return kind == Primitive; }
	bool getIsVector() const { return kind == Vector; }
	bool getIsMatrix() const { return kind == Matrix; }
	bool getIsStruct() const { return kind == Struct; }

private:
	TypeKind kind;
};

class IRPrimitiveType: public IRType
{
public:
	IRPrimitiveType(PrimitiveKind _kind): IRType(IRType::Primitive), kind(_kind) {}
	virtual ~IRPrimitiveType() override {}

	PrimitiveKind getPrimitiveKind() const { return kind; }

private:
	PrimitiveKind kind;
};

class IRVectorType: public IRType
{
public:
	IRVectorType(IRPrimitiveType *_base, int _dim): IRType(IRType::Vector), base(_base), dim(_dim) {}
	virtual ~IRVectorType() override {}

	IRPrimitiveType *getBaseType() const { return base; }
	int getDimension() const { return dim; }

private:
	IRPrimitiveType *base;
	int dim;
};

class IRMatrixType: public IRType
{
public:
	IRMatrixType(IRPrimitiveType *_base, int _rowdim, int _coldim): IRType(IRType::Matrix), base(_base), rowdim(_rowdim), coldim(_coldim) {}
	virtual ~IRMatrixType() override {}

	IRPrimitiveType *getBase() const { return base; }
	int getRowDimension() const { return rowdim; }
	int getColumnDimension() const { return coldim; }

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
