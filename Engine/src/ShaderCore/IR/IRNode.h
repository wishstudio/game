#pragma once

#include <memory>
#include <string>
#include <vector>

class IRType;
class IRNode
{
public:
	enum NodeKind
	{
		VariableDef,
		Constant,
		Variable,
		Unary,
		Binary,
		Field,
		Invoke,
		Assign,
		Return,
		List,
		Function,
	};
	IRNode(NodeKind _kind): kind(_kind) {}
	virtual ~IRNode() {}

	inline bool getIsConstant() const { return kind == Constant; }
	inline bool getIsUnary() const { return kind == Unary; }
	inline bool getIsBinary() const { return kind == Binary; }

private:
	NodeKind kind;
};

class IRVariableDef: public IRNode
{
public:
	enum VariableKind { Local, Parameter };
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

class IRVariable: public IRNode
{
public:
	IRVariable(IRVariableDef *_variable): IRNode(IRNode::Variable), variable(_variable) {}
	virtual ~IRVariable() override {}

private:
	IRVariableDef *variable;
};

class IRUnary: public IRNode
{
public:
	enum UnaryKind { Negate, Not };
	IRUnary(IRNode *_operand): IRNode(IRNode::Unary), operand(_operand) {}
	virtual ~IRUnary() override {}

	IRNode *getOperand() const { return operand.get(); }

private:
	std::unique_ptr<IRNode> operand;
};

class IRBinary: public IRNode
{
public:
	enum BinaryKind { Add, Sub, Mul, Div };
	IRBinary(IRNode *_lhs, IRNode *_rhs): IRNode(IRNode::Binary), lhs(_lhs), rhs(_rhs) {}
	virtual ~IRBinary() override {}

	IRNode *getLeft() const { return lhs.get(); }
	IRNode *getRight() const { return rhs.get(); }

private:
	std::unique_ptr<IRNode> lhs, rhs;
};

class IRField: public IRNode
{
public:
	IRField(IRNode *_object, const std::string &_field): IRNode(IRNode::Field), object(_object), field(_field) {}
	virtual ~IRField() override {}

	IRNode *getObject() const { return object.get(); }
	std::string getField() const { return field; }

private:
	std::unique_ptr<IRNode> object;
	std::string field;
};

class IRInvoke: public IRNode
{
public:
	IRInvoke(): IRNode(IRNode::Invoke) {}
	IRInvoke(IRNode *_object): IRNode(IRNode::Invoke), object(_object) {}
	virtual ~IRInvoke() override {}

	bool getIsStaticInvoke() const { return object == nullptr; }
	bool getIsFieleInvoke() const { return object != nullptr; }
	IRNode *getObject() const { return object.get(); }

private:
	std::unique_ptr<IRNode> object;
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

class IRFunction: public IRNode
{
public:
	IRFunction(const std::string &_name): IRNode(IRNode::Function), name(_name) {}
	virtual ~IRFunction() override {}

	std::string getName() const { return name; }

	IRList *getBody() const { return body.get(); }
	void setBody(IRList *_body) { body.reset(_body); }

	void addParameter(IRVariable *param) { parameters.push_back(std::unique_ptr<IRVariable>(param)); }
	int getParameterCount() const { return parameters.size(); }
	IRVariable *getParameter(int index) const { return parameters[index].get(); }

	void setReturn(IRVariable *_ret) { ret.reset(_ret); }
	IRVariable *getReturn() const { return ret.get(); }

private:
	std::string name;
	std::unique_ptr<IRList> body;
	std::vector<std::unique_ptr<IRVariable>> parameters;
	std::unique_ptr<IRVariable> ret;
};
