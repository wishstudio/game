#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class IRNode;
class SymbolTable final
{
public:
	SymbolTable();
	
	void enterScope();
	void leaveScope();

	bool add(const std::string &name, IRNode *symbol);
	IRNode *lookup(const std::string &name) const;

private:
	std::vector<std::unordered_map<std::string, IRNode *>> symbols;
};
