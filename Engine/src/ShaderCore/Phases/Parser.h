#pragma once

#include "../Context.h"
#include "../IR/IRNode.h"
#include "../IR/IRType.h"
#include "Lexer.h"

class Parser final
{
public:
	Parser() {}

	Context *parseShader(const std::string &source);

private:
	IRType *tryParseType();
	template <typename T>
	void parseSingleVariableDef(IRVariable::VariableKind kind, IRType *type, T callback);
	template <typename T>
	void parseVariableDef(IRVariable::VariableKind kind, IRType *type, T callback);

	IRValue *parseFunctionCall(IRValue *object, IRFunction *func);
	IRValue *parseElement();
	IRValue *parseUnaryOperator();
	IRValue *parseBinaryOperator(int precedence);
	inline IRValue *parseExpression() { return parseBinaryOperator(0); }
	IRList *parseStatements();

	Lexer *lexer;
	Context *ctx;
};
