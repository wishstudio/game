#pragma once

class IRNode
{
public:
	enum NodeKind
	{
		Type,
		Variable,
		Constant,
		VariableRef,
		Unary,
		Binary,
		FieldRef,
		Invoke,
		Assign,
		Return,
		List,
		Function,
	};
	IRNode(NodeKind _kind) : kind(_kind) {}
	virtual ~IRNode() {}

	inline bool getIsType() const { return kind == Type; }
	inline bool getIsVariable() const { return kind == Variable; }
	inline bool getIsConstant() const { return kind == Constant; }
	inline bool getIsVariableRef() const { return kind == VariableRef; }
	inline bool getIsUnary() const { return kind == Unary; }
	inline bool getIsBinary() const { return kind == Binary; }
	inline bool getIsFieldRef() const { return kind == FieldRef; }
	inline bool getIsAssign() const { return kind == Assign; }
	inline bool getIsReturn() const { return kind == Return; }
	inline bool getIsList() const { return kind == List; }
	inline bool getIsFunction() const { return kind == Function; }

	virtual bool getIsValue() const { return false; }
	virtual bool getIsLValue() const { return false; }

private:
	NodeKind kind;
};
