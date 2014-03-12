#include <cassert>

#include "D3D11Backend.h"

std::string D3D11Backend::getTypeName(IRType *type) const
{
	if (type->getIsPrimitive())
	{
		IRType::PrimitiveKind kind = ((IRPrimitiveType *) type)->getPrimitiveKind();
		switch (kind)
		{
		case IRType::Float:
			return "float";
		}
	}
	else if (type->getIsVector())
	{
		IRVectorType *vt = (IRVectorType *) type;
		return getTypeName(vt->getBaseType()) + std::to_string(vt->getDimension());
	}
	else if (type->getIsMatrix())
	{
		IRMatrixType *mt = (IRMatrixType *) type;
		return getTypeName(mt->getBaseType()) + std::to_string(mt->getRowDimension()) + "x" + std::to_string(mt->getColumnDimension());
	}
	else if (type->getIsStruct())
		return ((IRStructType *) type)->getName();
	assert(false);
	return std::string();
}

void D3D11Backend::compileValue(IRValue *value)
{
	if (value->getIsConstant())
	{
		assert(false);
	}
	else if (value->getIsVariableRef())
		ret += ((IRVariableRef *) value)->getVariable()->getName();
	else if (value->getIsUnary())
	{
		assert(false);
	}
	else if (value->getIsBinary())
	{
		IRBinary *b = (IRBinary *) value;
		ret += "(";
		compileValue(b->getLeft());
		switch (b->getOperator())
		{
		case IRBinary::Add: ret += "+"; break;
		case IRBinary::Sub: ret += "-"; break;
		case IRBinary::Mul: ret += "*"; break;
		case IRBinary::Div: ret += "/"; break;
		case IRBinary::Mod: ret += "%"; break;
		}
		compileValue(b->getRight());
		ret += ")";
	}
	else if (value->getIsFieldRef())
	{
		IRFieldRef *f = (IRFieldRef *) value;
		compileValue(f->getObject());
		ret += "." + f->getField();
	}
	else if (value->getIsInvoke())
	{
		IRInvoke *invoke = (IRInvoke *) value;
		ret += invoke->getFunction()->getName() + "(";
		for (int i = 0; i < invoke->getParameterCount(); i++)
		{
			if (i)
				ret += ", ";
			compileValue(invoke->getParameter(i));
		}
		ret += ")";
	}
}

void D3D11Backend::compileSingleVariableDef(IRVariable *varDef)
{
	ret += getTypeName(varDef->getType()) + " " + varDef->getName();
	if (varDef->getHasSemantic())
		ret += ": " + varDef->getSemantic();
}

void D3D11Backend::compileNode(IRNode *node)
{
	if (node->getIsType())
	{
		IRType *type = (IRType *) node;
		if (type->getIsStruct())
		{
			IRStructType *structType = (IRStructType *) type;
			ret += "struct " + structType->getName() + "\n";
			ret += "{\n";
			for (int i = 0; i < structType->getFieldCount(); i++)
			{
				IRType *fieldType = structType->getFieldType(i);
				std::string fieldName = structType->getFieldName(i), semantic = structType->getFieldSemantic(i);
				ret += getTypeName(fieldType) + " " + fieldName;
				if (!semantic.empty())
					ret += ": " + semantic;
				ret += ";\n";
			}
			ret += "}\n";
		}
	}
	else if (node->getIsAssign())
	{
		IRAssign *assign = (IRAssign *) node;
		compileValue(assign->getLeft());
		ret += "=";
		compileValue(assign->getRight());
		ret += ";\n";
	}
	else if (node->getIsReturn())
	{
		IRReturn *r = (IRReturn *) node;
		ret += "return ";
		compileValue(r->getValue());
		ret += ";\n";
	}
	else if (node->getIsList())
	{
		IRList *list = (IRList *) node;
		ret += "{\n";
		for (int i = 0; i < list->getNodeCount(); i++)
			compileNode(list->getNode(i));
		ret += "}\n";
	}
	else if (node->getIsVariable())
	{
		compileSingleVariableDef((IRVariable *) node);
		ret += ";\n";
	}
	else if (node->getIsFunction())
	{
		IRFunction *func = (IRFunction *) node;
		IRVariable *returnDef = func->getReturn();
		ret += getTypeName(returnDef->getType()) + " " + func->getName() + "(";
		for (int i = 0; i < func->getParameterCount(); i++)
		{
			if (i)
				ret += ", ";
			compileSingleVariableDef(func->getParameter(i));
		}
		ret += ")";
		if (returnDef->getHasSemantic())
			ret += ": " + returnDef->getSemantic();
		ret += "\n";
		compileNode(func->getBody());
	}
	else if (node->getIsValue())
	{
		compileValue((IRValue *) node);
		ret += ";\n";
	}
	else
		assert(false);
}

std::string D3D11Backend::compile(Context *context)
{
	ret.clear();
	for (const std::unique_ptr<IRNode> &node: context->globalDefs)
		compileNode(node.get());
	return ret;
}
