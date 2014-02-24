#pragma once

#include <string>

class IRNode;
class Parser final
{
public:
	Parser();

	IRNode *parse(const std::string &source);

private:
};
