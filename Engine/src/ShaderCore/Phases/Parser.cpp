#include <cctype>

#include "Parser.h"

void Parser::getCh()
{
	if (pointer == source.size())
		ch = -1;
	else
		ch = source[pointer++];
}

void Parser::getToken()
{
	while (isspace(ch))
		getCh();
	if (ch == -1)
	{
		token = tkEOF;
		return;
	}
	if (isalpha(ch) || ch == '_')
	{
		tokenIdent = ch;
		getCh();
		while (isalpha(ch) || isdigit(ch) || ch == '_')
		{
			tokenIdent.push_back(ch);
			getCh();
		}
		if (tokenIdent == "return")
			token = tkReturn;
		else
			token = tkIdentifier;
		return;
	}
	switch (ch)
	{
	case '+':
		token = tkAdd;
		return;

	case '-':
		token = tkSub;
		return;

	case '*':
		token = tkMul;
		return;

	case '/':
		token = tkDiv;
		return;

	case '%':
		token = tkMod;
		return;

	case '=':
		token = tkAssign;
		return;

	case '.':
		token = tkDot;
		return;

	case ',':
		token = tkComma;
		return;

	case ':':
		token = tkColon;
		return;

	case ';':
		token = tkSemicolon;
		return;

	case '(':
		token = tkPOpen;
		return;

	case ')':
		token = tkPClose;
		return;

	case '[':
		token = tkSOpen;
		return;

	case ']':
		token = tkSClose;
		return;

	case '{':
		token = tkBOpen;
		return;

	case '}':
		token = tkBClose;
		return;

	default:
		token = tkERROR;
		return;
	}
}

Context *Parser::parseShader(const std::string &_source)
{
	ctx = new Context();

	source = _source;
	pointer = 0;

	getCh();
	getToken();

	return ctx;
}
