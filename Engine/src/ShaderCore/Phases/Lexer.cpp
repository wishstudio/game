#include "Lexer.h"

Lexer::Lexer(const std::string &_source)
	: source(_source)
{
	ctx.pointer = 0;

	nextChar();
	nextToken();
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
		ctx.token = Error;;
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
		else
			ctx.token = Identifier;
		return;
	}
	switch (ctx.ch)
	{
	case '+':
		ctx.token = Add;
		return;

	case '-':
		ctx.token = Sub;
		return;

	case '*':
		ctx.token = Mul;
		return;

	case '/':
		ctx.token = Div;
		return;

	case '%':
		ctx.token = Mod;
		return;

	case '=':
		ctx.token = Assign;
		return;

	case '.':
		ctx.token = Dot;
		return;

	case ',':
		ctx.token = Comma;
		return;

	case ':':
		ctx.token = Colon;
		return;

	case ';':
		ctx.token = Semicolon;
		return;

	case '(':
		ctx.token = POpen;
		return;

	case ')':
		ctx.token = PClose;
		return;

	case '[':
		ctx.token = SOpen;
		return;

	case ']':
		ctx.token = SClose;
		return;

	case '{':
		ctx.token = BOpen;
		return;

	case '}':
		ctx.token = BClose;
		return;

	default:
		ctx.token = Error;
		return;
	}
}
