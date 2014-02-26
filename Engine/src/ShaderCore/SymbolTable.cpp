#include "SymbolTable.h"

SymbolTable::SymbolTable()
{
	/* Automatically creates a global scope */
	enterScope();
}

void SymbolTable::enterScope()
{
	symbols.push_back({});
}

void SymbolTable::leaveScope()
{
	symbols.pop_back();
}

bool SymbolTable::add(const std::string &name, IRNode *symbol)
{
	if (symbols.back().find(name) != symbols.back().end())
		return false;
	symbols.back().insert({ name, symbol });
	return true;
}

IRNode *SymbolTable::lookup(const std::string &name) const
{
	for (auto it = symbols.crbegin(); it != symbols.crend(); it++)
	{
		auto l = it->find(name);
		if (l != it->end())
			return l->second;
	}
	return nullptr;
}
