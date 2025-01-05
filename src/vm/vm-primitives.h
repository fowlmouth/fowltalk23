#pragma once

#include "image.h"
#include "object.h"
#include "primitives.h"

#include <memory>

class VirtualMachine;
class VMPrimitiveInterface;

struct PrimitiveFunction
{
  using function_t = int(*)(VMPrimitiveInterface& vm, int argc, oop* argv);

  function_t fn = nullptr;
  void* dylib = nullptr;
  const char* symbol_name = nullptr;
};

class PrimitiveFunctionSet
{
  int primitive_count, primitive_capacity;
  std::unique_ptr< PrimitiveFunction[] > primitive_functions;

public:
  PrimitiveFunctionSet(int capacity);

  void register_primitive(Image& image, primitive_id_t pid, PrimitiveFunction::function_t fn, const char* selector, void* dylib, const char* symbol_name);
  const PrimitiveFunction* get(primitive_id_t pid) const;

  void load_defaults(Image& image);
};
