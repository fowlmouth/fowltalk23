#pragma once

#include "object.h"
#include "primitives.h"

class VirtualMachine;

struct PrimitiveFunction
{
  using function_t = int(*)(VirtualMachine& , int argc, oop* argv);

  function_t fn;
  void* dylib;
  const char* symbol_name;
};

void load_default_primitives(VirtualMachine& vm);
