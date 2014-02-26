#include "Context.h"
#include "IR/IRType.h"

Context::Context()
{
	IRPrimitiveType *floatType = new IRPrimitiveType(IRType::Float);
	symbolTable.add("float", floatType);
	symbolTable.add("float2", new IRVectorType(floatType, 2));
	symbolTable.add("float3", new IRVectorType(floatType, 3));
	symbolTable.add("float4", new IRVectorType(floatType, 4));
	symbolTable.add("float4x4", new IRMatrixType(floatType, 4, 4));
}
