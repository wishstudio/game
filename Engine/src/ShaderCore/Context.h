#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class IRType;
class Context
{
public:
	Context();

	std::unordered_map<std::string, std::unique_ptr<IRType>> types;
};
