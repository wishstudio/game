#include "IRType.h"
#include "IRNode.h"

IRFunction *IRTexture2D::getFunction(const std::string &name)
{
	if (name == "Sample")
	{
		/* TODO: Temporary workaround */
		return new IRFunction("Sample");
	}
	else
		return nullptr;
}
