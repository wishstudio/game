#pragma once

#include <string>

class Lexer
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
		Add,
		Sub,
		Mul,
		Div,
		Mod,
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
};
