#pragma once

#include "libfowl.h"
#include "vm_spec.h"
#include "vm-primitives.h"

class VirtualMachine
{
  struct ExecutionContext
  {
    int lexical_parent, sp;
    int ip;
    oop method;

    ExecutionContext()
    {
    }

    object_array bytecode_slots(Memory& mem) const
    {
      return (object_array)mem.ptr(mem.vtable(method)->bytecode());
    }
  };

  Image& image;

  int primitive_count, primitive_capacity;
  std::unique_ptr< PrimitiveFunction[] > primitive_functions;

  int frame_ptr, frame_capacity;
  std::unique_ptr< ExecutionContext[] > frames;

  int stack_capacity;
  std::unique_ptr< oop[] > stack;

  vm_instruction_t* ip;
  oop* sp;
  object_array immediates;

protected:
  void enter_method(oop method);
  ExecutionContext* fp() const;
  void entered_frame();

public:
  VirtualMachine(Image& image, oop entrypoint_method);
  bool lookup(oop receiver, oop selector, oop& result) const;
  void run(int ticks = 1024);

  void register_primitive( PrimitiveFunction::function_t fn, const char* selector, void* dylib, const char* symbol_name);

};
