#include "Context.h"
#include "IR/IRType.h"

Context::Context()
{
	IRPrimitiveType *floatType = new IRPrimitiveType(IRType::Float);
	types.emplace("float", floatType);
	types.emplace("float2", new IRVectorType(floatType, 2));
	types.emplace("float3", new IRVectorType(floatType, 3));
	types.emplace("float4", new IRVectorType(floatType, 4));
	types.emplace("float4x4", new IRMatrixType(floatType, 4, 4));
}
