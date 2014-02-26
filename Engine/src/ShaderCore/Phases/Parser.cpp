#include <cassert>
#include <cctype>

#include "Parser.h"

IRType *Parser::tryParseType()
{
	if (lexer->getToken() == Lexer::Identifier)
	{
		auto it = ctx->types.find(lexer->getTokenIdentifier());
		if (it != ctx->types.end())
			return it->second.get();
	}
	return nullptr;
}

template <typename T>
void Parser::parseVariableDef(IRVariableDef::VariableKind kind, IRType *type, T callback)
{
	for (;;)
	{
		std::string name, semantic;

		assert(lexer->getToken() == Lexer::Identifier);
		name = lexer->getTokenIdentifier();
		lexer->nextToken();
		if (lexer->getToken() == Lexer::Colon)
		{
			lexer->nextToken();
			assert(lexer->getToken() == Lexer::Identifier);
			semantic = lexer->getTokenIdentifier();
			lexer->nextToken();
		}
		if (lexer->getToken() == Lexer::Assign)
		{
			lexer->nextToken();
			/* TODO */
		}
		callback(new IRVariableDef(kind, type, name, semantic), nullptr);
		if (lexer->getToken() == Lexer::Comma)
		{
			lexer->nextToken();
			continue;
		}
	}
}

Context *Parser::parseShader(const std::string &source)
{
	ctx = new Context();

	/* Prepare initial lexer/tokenizer context */
	lexer = new Lexer(source);

	while (lexer->getToken() != Lexer::Eof)
	{
		IRType *type = tryParseType();
		if (type != nullptr)
		{
			parseVariableDef(IRVariableDef::Global, type, [&](IRVariableDef *varDef, IRNode *initialValue) {
				assert(initialValue == nullptr);
				ctx->globalVars.push_back(std::unique_ptr<IRVariableDef>(varDef));
			});
			assert(lexer->getToken() == Lexer::Semicolon);
			lexer->nextToken();
		}
		else
		{
		}
	}

	delete lexer;

	return ctx;
}
