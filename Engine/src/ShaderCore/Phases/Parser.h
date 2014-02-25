#pragma once

#include "../Context.h"

class IRNode;
class IRType;
class Parser final
{
public:
	Parser() {}

	Context *parseShader(const std::string &source);

private:
	/* Reader */
	std::string source;
	char ch;
	int pointer;
	void getCh();
	
	/* Tokenizer */
	enum TokenType
	{
		tkEOF,
		tkERROR,
		tkReturn,
		tkIdentifier,
		tkAdd,
		tkSub,
		tkMul,
		tkDiv,
		tkMod,
		tkAssign,
		tkDot,
		tkComma,
		tkColon,
		tkSemicolon,
		tkPOpen,
		tkPClose,
		tkSOpen,
		tkSClose,
		tkBOpen,
		tkBClose,
	};
	TokenType token;
	std::string tokenIdent;
	void getToken();
	
	/* Parser */
	Context *ctx;
};
