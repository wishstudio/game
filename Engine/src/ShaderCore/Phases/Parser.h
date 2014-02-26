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
	void parseSingleVariableDef(IRVariableDef::VariableKind kind, IRType *type, T callback);
	template <typename T>
	void parseVariableDef(IRVariableDef::VariableKind kind, IRType *type, T callback);
	IRList *parseStatementList();

	Lexer *lexer;
	Context *ctx;
};
