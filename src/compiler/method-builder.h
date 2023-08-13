#pragma once

#include "parser.h"

class MethodBuilder
{
  MethodBuilder* parent_;

public:
  MethodBuilder();
  MethodBuilder(MethodBuilder* parent);

};

