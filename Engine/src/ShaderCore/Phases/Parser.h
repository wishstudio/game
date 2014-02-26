#pragma once

#include "../Context.h"
#include "../IR/IRNode.h"
#include "../IR/IRType.h"
#include "Lexer.h"

class Parser final
{
public:
	Parser() {}

	IRType *tryParseType();
	template <typename T>
	void parseVariableDef(IRVariableDef::VariableKind kind, IRType *type, T callback);
	Context *parseShader(const std::string &source);

private:
	/* Parser */
	Lexer *lexer;
	Context *ctx;
};
