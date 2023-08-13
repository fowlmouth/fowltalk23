#include "method-builder.h"

MethodBuilder::MethodBuilder()
: MethodBuilder(nullptr)
{
}

MethodBuilder::MethodBuilder(MethodBuilder* parent)
: parent_(parent)
{
}
