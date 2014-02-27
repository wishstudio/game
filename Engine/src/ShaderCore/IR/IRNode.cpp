#include "IRNode.h"

static int binaryOperatorPrecedence[] = {
	0, 0,
	1, 1, 1,
};
int IRBinary::getPrecedence(IRBinary::BinaryKind kind)
{
	return binaryOperatorPrecedence[kind];
}
