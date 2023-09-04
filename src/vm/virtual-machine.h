#pragma once

#include "libfowl.h"
#include "vm_spec.h"

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

  Memory& mem;
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
  VirtualMachine(Memory& mem, oop entrypoint_method);
  bool lookup(oop receiver, oop selector, oop& result) const;
  void run(int ticks = 1024);

};
