#include "Context.h"
#include "IR/IRType.h"

Context::Context()
{
	IRPrimitiveType *floatType = new IRPrimitiveType(IRType::Float);
	types.emplace("float", std::unique_ptr<IRType>(floatType));
	types.emplace("float2", std::unique_ptr<IRType>(new IRVectorType(floatType, 2)));
	types.emplace("float3", std::unique_ptr<IRType>(new IRVectorType(floatType, 3)));
	types.emplace("float4", std::unique_ptr<IRType>(new IRVectorType(floatType, 4)));
	types.emplace("float4x4", std::unique_ptr<IRType>(new IRMatrixType(floatType, 4, 4)));
}
