#include "Lexer.h"

Lexer::Lexer(const std::string &_source)
	: source(_source)
{
	ctx.pointer = 0;

	nextChar();
	nextToken();
}

static IRBinary::BinaryKind binaryTokenMap[] = {
	IRBinary::Add,
	IRBinary::Sub,
	IRBinary::Mul,
	IRBinary::Div,
	IRBinary::Mod,
};
IRBinary::BinaryKind Lexer::translateBinaryToken(Lexer::TokenType token)
{
	return binaryTokenMap[token - Lexer::_BINARY_TOKEN_BEGIN - 1];
}

Lexer::LexerContext *Lexer::saveContext() const
{
	return new LexerContext(ctx);
}

void Lexer::restoreContext(Lexer::LexerContext *savedContext)
{
	/* TODO: Define move operators for LexerContext */
	ctx = std::move(*savedContext);
	delete savedContext;
}

void Lexer::nextChar()
{
	if (ctx.pointer == source.size())
		ctx.ch = -1;
	else
		ctx.ch = source[ctx.pointer++];
}

void Lexer::nextToken()
{
	while (isspace(ctx.ch))
		nextChar();
	if (ctx.ch == -1)
	{
		ctx.token = Eof;
		return;
	}
	if (isalpha(ctx.ch) || ctx.ch == '_')
	{
		ctx.tokenIdentifier = ctx.ch;
		nextChar();
		while (isalpha(ctx.ch) || isdigit(ctx.ch) || ctx.ch == '_')
		{
			ctx.tokenIdentifier.push_back(ctx.ch);
			nextChar();
		}
		if (ctx.tokenIdentifier == "struct")
			ctx.token = Struct;
		else if (ctx.tokenIdentifier == "return")
			ctx.token = Return;
		else if (ctx.tokenIdentifier == "cbuffer")
			ctx.token = CBuffer;
		else
			ctx.token = Identifier;
		return;
	}
	switch (ctx.ch)
	{
	case '+':
		ctx.token = Add;
		nextChar();
		return;

	case '-':
		ctx.token = Sub;
		nextChar();
		return;

	case '*':
		ctx.token = Mul;
		nextChar();
		return;

	case '/':
		ctx.token = Div;
		nextChar();
		return;

	case '%':
		ctx.token = Mod;
		nextChar();
		return;

	case '=':
		ctx.token = Assign;
		nextChar();
		return;

	case '.':
		ctx.token = Dot;
		nextChar();
		return;

	case ',':
		ctx.token = Comma;
		nextChar();
		return;

	case ':':
		ctx.token = Colon;
		nextChar();
		return;

	case ';':
		ctx.token = Semicolon;
		nextChar();
		return;

	case '(':
		ctx.token = POpen;
		nextChar();
		return;

	case ')':
		ctx.token = PClose;
		nextChar();
		return;

	case '[':
		ctx.token = SOpen;
		nextChar();
		return;

	case ']':
		ctx.token = SClose;
		nextChar();
		return;

	case '{':
		ctx.token = BOpen;
		nextChar();
		return;

	case '}':
		ctx.token = BClose;
		nextChar();
		return;

	default:
		ctx.token = Error;
		nextChar();
		return;
	}
}
