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
void Parser::parseSingleVariableDef(IRVariable::VariableKind kind, IRType *type, T callback)
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
	IRValue *initialValue = nullptr;
	if (lexer->getToken() == Lexer::Assign)
	{
		lexer->nextToken();
		initialValue = parseExpression();
	}
	callback(new IRVariable(kind, type, name, semantic), initialValue);
}

template <typename T>
void Parser::parseVariableDef(IRVariable::VariableKind kind, IRType *type, T callback)
{
	for (;;)
	{
		parseSingleVariableDef(kind, type, callback);
		if (lexer->getToken() == Lexer::Comma)
		{
			lexer->nextToken();
			continue;
		}
		else
			break;
	}
}

IRValue *Parser::parseElement()
{
	switch (lexer->getToken())
	{
	case Lexer::POpen:
	{
		IRValue *value;
		lexer->nextToken();
		value = parseExpression();
		assert(lexer->getToken() == Lexer::PClose);
		lexer->nextToken();
		return value;
	}

	case Lexer::Identifier:
	{
		IRNode *node = ctx->symbolTable.lookup(lexer->getTokenIdentifier());
		lexer->nextToken();
		if (node->getIsVariable())
			return new IRVariableRef((IRVariable *) node);
		else if (node->getIsFunction())
		{
			/* Function invocation */
			assert(lexer->getToken() == Lexer::POpen);
			lexer->nextToken();
			IRFunction *func = (IRFunction *) node;
			IRInvoke *invokeValue = new IRInvoke(func);
			if (lexer->getToken() != Lexer::PClose)
			{
				for (;;)
				{
					IRValue *parameter = parseExpression();
					invokeValue->addParameter(parameter);
					if (lexer->getToken() == Lexer::Comma)
					{
						lexer->nextToken();
						continue;
					}
					else
						break;
				}
			}
			assert(lexer->getToken() == Lexer::PClose);
			lexer->nextToken();
			return invokeValue;
		}
		else
			assert(false);
	}

	default:
		assert(false);
	}
	return nullptr;
}

IRValue *Parser::parseUnaryOperator()
{
	IRValue *value = parseElement();
	while (lexer->getToken() == Lexer::Dot)
	{
		lexer->nextToken();
		assert(lexer->getToken() == Lexer::Identifier);
		value = new IRFieldRef(value, lexer->getTokenIdentifier());
		lexer->nextToken();
	}
	return value;
}

IRValue *Parser::parseBinaryOperator(int precedence)
{
	IRValue *left = parseUnaryOperator();
	IRBinary::BinaryKind op;
	while (lexer->isBinaryToken(lexer->getToken()) &&
		(op = lexer->translateBinaryToken(lexer->getToken()), IRBinary::getPrecedence(op) >= precedence))
	{
		/* Parse binary operator with higher precedence */
		lexer->nextToken();
		IRValue *right = parseBinaryOperator(precedence + 1);
		left = new IRBinary(op, left, right);
	}
	return left;
}

IRList *Parser::parseStatements()
{
	IRList *stmts = new IRList();

	for (;;)
	{
		switch (lexer->getToken())
		{
		case Lexer::BOpen:
			lexer->nextToken();
			ctx->symbolTable.enterScope();
			stmts->addNode(parseStatements());
			ctx->symbolTable.leaveScope();
			assert(lexer->getToken() == Lexer::BClose);
			lexer->nextToken();
			continue;

		case Lexer::BClose:
			return stmts;

		case Lexer::Return:
		{
			lexer->nextToken();
			/* TODO: Check void type */
			IRValue *value = parseExpression();
			stmts->addNode(new IRReturn(value));
			break;
		}

		default:
		{
			IRType *type = tryParseType();
			if (type != nullptr)
			{
				/* Variable definition */
				parseVariableDef(IRVariable::Local, type, [&](IRVariable *varDef, IRValue *initialValue) {
					assert(initialValue == nullptr);
					stmts->addNode(varDef);
					assert(ctx->symbolTable.add(varDef->getName(), varDef));
				});
			}
			else
			{
				IRValue *value = parseExpression();
				if (lexer->getToken() == Lexer::Assign)
				{
					assert(value->getIsLValue());
					lexer->nextToken();
					IRValue *rhs = parseExpression();
					stmts->addNode(new IRAssign(value, rhs));
				}
				else
					stmts->addNode(value);
			}
		}
		} /* switch(lexer->getToken()) */
		assert(lexer->getToken() == Lexer::Semicolon);
		lexer->nextToken();
	}
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
				/* Parse parameter list */
				for (;;)
				{
					IRType *type = tryParseType();
					assert(type != nullptr);
					parseSingleVariableDef(IRVariable::Parameter, type, [&](IRVariable *varDef, IRValue *initialValue) {
						assert(initialValue == nullptr);
						func->addParameter(varDef);
						assert(ctx->symbolTable.add(varDef->getName(), varDef));
					});
					if (lexer->getToken() == Lexer::Comma)
						lexer->nextToken();
					else
						break;
				}
				assert(lexer->getToken() == Lexer::PClose);
				lexer->nextToken();
				std::string semantic;
				if (lexer->getToken() == Lexer::Colon)
				{
					lexer->nextToken();
					assert(lexer->getToken() == Lexer::Identifier);
					semantic = lexer->getTokenIdentifier();
					lexer->nextToken();
				}
				func->setReturn(new IRVariable(IRVariable::Return, returnType, std::string(), semantic));
				assert(lexer->getToken() == Lexer::BOpen);
				lexer->nextToken();
				func->setBody(parseStatements());
				assert(lexer->getToken() == Lexer::BClose);
				lexer->nextToken();

				ctx->symbolTable.leaveScope();
				ctx->globalDefs.push_back(std::unique_ptr<IRNode>(func));
				assert(ctx->symbolTable.add(func->getName(), func));
			}
			else
			{
				/* Global variable definition */
				parseVariableDef(IRVariable::Global, type, [&](IRVariable *varDef, IRValue *initialValue) {
					assert(initialValue == nullptr);
					ctx->globalDefs.push_back(std::unique_ptr<IRNode>(varDef));
					assert(ctx->symbolTable.add(varDef->getName(), varDef));
				});
				assert(lexer->getToken() == Lexer::Semicolon);
				lexer->nextToken();
			}
		}
		else if (lexer->getToken() == Lexer::Struct)
		{
			lexer->nextToken();
			assert(lexer->getToken() == Lexer::Identifier);
			IRStructType *structType = new IRStructType(lexer->getTokenIdentifier());
			lexer->nextToken();
			assert(lexer->getToken() == Lexer::BOpen);
			lexer->nextToken();
			for (;;)
			{
				IRType *type = tryParseType();
				if (type == nullptr)
					break;
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
					structType->addField(type, name, semantic);
					if (lexer->getToken() == Lexer::Comma)
					{
						lexer->nextToken();
						continue;
					}
					else
						break;
				}
				assert(lexer->getToken() == Lexer::Semicolon);
				lexer->nextToken();
				if (lexer->getToken() == Lexer::Eof || lexer->getToken() == Lexer::Error || lexer->getToken() == Lexer::BClose)
					break;
			}
			ctx->symbolTable.add(structType->getName(), structType);
			ctx->globalDefs.push_back(std::unique_ptr<IRNode>(structType));
			assert(lexer->getToken() == Lexer::BClose);
			lexer->nextToken();
			assert(lexer->getToken() == Lexer::Semicolon);
			lexer->nextToken();
		}
		else if (lexer->getToken() == Lexer::CBuffer)
		{
			lexer->nextToken();
			assert(lexer->getToken() == Lexer::Identifier);
			IRCBuffer *cbuffer = new IRCBuffer(lexer->getTokenIdentifier());
			lexer->nextToken();
			assert(lexer->getToken() == Lexer::BOpen);
			lexer->nextToken();
			for (;;)
			{
				IRType *type = tryParseType();
				if (type == nullptr)
					break;
				parseVariableDef(IRVariable::CBuffer, type, [&](IRVariable *varDef, IRValue *initialValue) {
					assert(initialValue == nullptr);
					cbuffer->addVariable(varDef);
					ctx->symbolTable.add(varDef->getName(), varDef);
				});
				assert(lexer->getToken() == Lexer::Semicolon);
				lexer->nextToken();
			}
			ctx->globalDefs.push_back(std::unique_ptr<IRNode>(cbuffer));
			assert(lexer->getToken() == Lexer::BClose);
			lexer->nextToken();
		}
		else
			assert(false);
	}

	delete lexer;

	return ctx;
}
