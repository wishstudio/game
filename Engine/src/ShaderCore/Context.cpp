#include "Context.h"
#include "IR/IRType.h"

Context::Context()
{
	/* TODO: Temporary workarounds */
	IRPrimitiveType *floatType = new IRPrimitiveType(IRType::Float);
	symbolTable.add("float", floatType);
	symbolTable.add("float2", new IRVectorType(floatType, 2));
	symbolTable.add("float3", new IRVectorType(floatType, 3));
	symbolTable.add("float4", new IRVectorType(floatType, 4));
	symbolTable.add("float4x4", new IRMatrixType(floatType, 4, 4));

	IRFunction *func = new IRFunction("mul");
	func->addParameter(new IRVariable(IRVariable::Parameter, floatType, "var1"));
	func->addParameter(new IRVariable(IRVariable::Parameter, floatType, "var2"));
	symbolTable.add("mul", func);
}
