#pragma once

#include <memory>
#include <string>
#include <vector>

#include "IRBase.h"

class IRType;
class IRVariableDef;
class IRFunction;
class IRValue: public IRNode
{
public:
	IRValue(IRNode::NodeKind kind): IRNode(kind) {}

	virtual bool getIsValue() const override { return true; }
};

class IRVariable: public IRValue
{
public:
	IRVariable(IRVariableDef *_variable): IRValue(IRNode::Variable), variable(_variable) {}
	virtual ~IRVariable() override {}

	virtual bool getIsLValue() const override { return true; }

private:
	IRVariableDef *variable;
};

class IRField : public IRValue
{
public:
	IRField(IRValue *_object, const std::string &_field) : IRValue(IRNode::Field), object(_object), field(_field) {}
	virtual ~IRField() override {}

	IRValue *getObject() const { return object.get(); }
	std::string getField() const { return field; }

	virtual bool getIsLValue() const override { return true; }

private:
	std::unique_ptr<IRValue> object;
	std::string field;
};

class IRUnary: public IRValue
{
public:
	enum UnaryKind { Negate, Not };
	IRUnary(IRValue *_operand): IRValue(IRNode::Unary), operand(_operand) {}
	virtual ~IRUnary() override {}

	IRValue *getOperand() const { return operand.get(); }

private:
	std::unique_ptr<IRValue> operand;
};

class IRBinary: public IRValue
{
public:
	enum BinaryKind { Add, Sub, Div, Mul, Mod };
	IRBinary(BinaryKind _op, IRValue *_lhs, IRValue *_rhs): IRValue(IRNode::Binary), op(_op), lhs(_lhs), rhs(_rhs) {}
	virtual ~IRBinary() override {}

	static int getPrecedence(BinaryKind kind);

	BinaryKind getOperator() const { return op; }
	IRValue *getLeft() const { return lhs.get(); }
	IRValue *getRight() const { return rhs.get(); }

private:
	BinaryKind op;
	std::unique_ptr<IRValue> lhs, rhs;
};

class IRInvoke: public IRValue
{
public:
	IRInvoke(IRFunction *_func): IRValue(IRNode::Invoke), func(_func) {}
	virtual ~IRInvoke() override {}

	IRFunction *getFunction() const { return func.get(); }

	void addParameter(IRValue *value) { parameters.push_back(std::unique_ptr<IRValue>(value)); }
	int getParameterCount() const { return parameters.size(); }
	IRValue *getParameter(int index) const { return parameters[index].get(); }

private:
	std::unique_ptr<IRFunction> func;
	std::vector<std::unique_ptr<IRValue>> parameters;
};

class IRAssign: public IRNode
{
public:
	IRAssign(IRNode *_lhs, IRNode *_rhs): IRNode(IRNode::Assign), lhs(_lhs), rhs(_rhs) {}
	virtual ~IRAssign() override {}

	IRNode *getLeft() const { return lhs.get(); }
	IRNode *getRight() const { return rhs.get(); }

private:
	std::unique_ptr<IRNode> lhs, rhs;
};

class IRReturn: public IRNode
{
public:
	IRReturn(IRNode *_object): IRNode(IRNode::Return), object(_object) {}
	virtual ~IRReturn() override {}

	IRNode *getObject() const { return object.get(); }

private:
	std::unique_ptr<IRNode> object;
};

class IRList: public IRNode
{
public:
	IRList(): IRNode(IRNode::List) {}
	virtual ~IRList() override {}

	void addNode(IRNode *node) { nodes.push_back(std::unique_ptr<IRNode>(node)); }
	int getNodeCount() const { return nodes.size(); }
	IRNode *getNode(int index) const { return nodes[index].get(); }

private:
	std::vector<std::unique_ptr<IRNode>> nodes;
};

class IRVariableDef : public IRNode
{
public:
	enum VariableKind { Global, Local, Parameter, Return };
	IRVariableDef(VariableKind _kind, IRType *_type, const std::string &_name = std::string(), const std::string &_semantic = std::string())
		: IRNode(VariableDef), kind(_kind), type(_type), name(_name), semantic(_semantic) {}

	VariableKind getKind() const { return kind; }
	bool getIsLocal() const { return kind == Local; }
	bool getIParameter() const { return kind == Parameter; }

	bool getHasName() const { return !name.empty(); }
	std::string getName() const { return name; }
	bool getHasSemantic() const { return !semantic.empty(); }
	std::string getSemantic() const { return semantic; }
	IRType *getType() const { return type; }

private:
	VariableKind kind;
	IRType *type;
	std::string name, semantic;
};

class IRFunction: public IRNode
{
public:
	IRFunction(const std::string &_name): IRNode(IRNode::Function), name(_name) {}
	virtual ~IRFunction() override {}

	std::string getName() const { return name; }

	IRList *getBody() const { return body.get(); }
	void setBody(IRList *_body) { body.reset(_body); }

	void addParameter(IRVariableDef *param) { parameters.push_back(std::unique_ptr<IRVariableDef>(param)); }
	int getParameterCount() const { return parameters.size(); }
	IRVariableDef *getParameter(int index) const { return parameters[index].get(); }

	void setReturn(IRVariableDef *_ret) { ret.reset(_ret); }
	IRVariableDef *getReturn() const { return ret.get(); }

private:
	std::string name;
	std::unique_ptr<IRList> body;
	std::vector<std::unique_ptr<IRVariableDef>> parameters;
	std::unique_ptr<IRVariableDef> ret;
};
