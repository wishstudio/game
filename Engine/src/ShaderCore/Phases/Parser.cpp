#include <cassert>
#include <cctype>

#include "Parser.h"

IRType *Parser::tryParseType()
{
	if (lexer->getToken() == Lexer::Identifier)
	{
		IRNode *node = ctx->symbolTable.lookup(lexer->getTokenIdentifier());
		if (node != nullptr && node->getIsType())
		{
			lexer->nextToken();
			return (IRType *) node;
		}
	}
	return nullptr;
}

template <typename T>
void Parser::parseSingleVariableDef(IRVariableDef::VariableKind kind, IRType *type, T callback)
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
}

template <typename T>
void Parser::parseVariableDef(IRVariableDef::VariableKind kind, IRType *type, T callback)
{
	for (;;)
	{
		parseSingleVariableDef(kind, type, callback);
		if (lexer->getToken() == Lexer::Comma)
		{
			lexer->nextToken();
			continue;
		}
	}
}

IRList *Parser::parseStatementList()
{
	IRList *list = new IRList();
	return list;
}

Context *Parser::parseShader(const std::string &source)
{
	ctx = new Context();
	lexer = new Lexer(source);

	while (lexer->getToken() != Lexer::Eof)
	{
		IRType *type = tryParseType();
		if (type != nullptr)
		{
			assert(lexer->getToken() == Lexer::Identifier);
			bool isFunctionDef = false;
			auto lc = lexer->saveContext();
			lexer->nextToken();
			if (lexer->getToken() == Lexer::POpen)
				isFunctionDef = true;
			lexer->restoreContext(lc);

			if (isFunctionDef)
			{
				/* Function definition */
				IRType *returnType = type;
				ctx->symbolTable.enterScope();

				IRFunction *func = new IRFunction(lexer->getTokenIdentifier());
				lexer->nextToken();
				assert(lexer->getToken() == Lexer::POpen);
				lexer->nextToken();
				/* Parse variable list */
				while (lexer->getToken() != Lexer::PClose)
				{
					IRType *type = tryParseType();
					assert(type != nullptr);
					parseSingleVariableDef(IRVariableDef::Parameter, type, [&](IRVariableDef *varDef, IRNode *initialValue) {
						assert(initialValue == nullptr);
						func->addParameter(varDef);
						assert(ctx->symbolTable.add(varDef->getName(), varDef));
					});
				}
				lexer->nextToken();
				std::string semantic;
				if (lexer->getToken() == Lexer::Colon)
				{
					lexer->nextToken();
					assert(lexer->getToken() == Lexer::Identifier);
					semantic = lexer->getTokenIdentifier();
					lexer->nextToken();
				}
				func->setReturn(new IRVariableDef(IRVariableDef::Return, returnType, std::string(), semantic));
				assert(lexer->getToken() == Lexer::BOpen);
				lexer->getToken();
				func->setBody(parseStatementList());
				assert(lexer->getToken() == Lexer::BClose);
				lexer->getToken();

				ctx->symbolTable.leaveScope();
				ctx->globalFuncs.push_back(std::unique_ptr<IRFunction>(func));
				assert(ctx->symbolTable.add(func->getName(), func));
			}
			else
			{
				/* Global variable definition */
				parseVariableDef(IRVariableDef::Global, type, [&](IRVariableDef *varDef, IRNode *initialValue) {
					assert(initialValue == nullptr);
					ctx->globalVars.push_back(std::unique_ptr<IRVariableDef>(varDef));
					assert(ctx->symbolTable.add(varDef->getName(), varDef));
				});
				assert(lexer->getToken() == Lexer::Semicolon);
				lexer->nextToken();
			}
		}
		else
		{
		}
	}

	delete lexer;

	return ctx;
}
