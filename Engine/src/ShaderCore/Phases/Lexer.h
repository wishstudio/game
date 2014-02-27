#pragma once

#include <string>

#include "../IR/IRNode.h"

class Lexer final
{
public:
	Lexer(const std::string &source);

	/* Lexer */
	enum TokenType
	{
		Eof,
		Error,
		Struct,
		Return,
		Identifier,
		_BINARY_TOKEN_BEGIN,
		Add,
		Sub,
		Mul,
		Div,
		Mod,
		_BINARY_TOKEN_END,
		Assign,
		Dot,
		Comma,
		Colon,
		Semicolon,
		POpen,
		PClose,
		SOpen,
		SClose,
		BOpen,
		BClose,
	};
	
	static bool isBinaryToken(TokenType token) { return token > _BINARY_TOKEN_BEGIN && token < _BINARY_TOKEN_END; }
	static IRBinary::BinaryKind translateBinaryToken(TokenType token);

	void nextToken();
	TokenType getToken() const { return ctx.token; }
	std::string getTokenIdentifier() const { return ctx.tokenIdentifier; }
	
private:
	void nextChar();

	struct LexerContext
	{
		char ch;
		int pointer;
		TokenType token;
		std::string tokenIdentifier;
	};
	std::string source;
	LexerContext ctx;

public:
	LexerContext *saveContext() const;
	void restoreContext(LexerContext *savedContext);
};
