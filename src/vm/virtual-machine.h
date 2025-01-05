#pragma once

#include "libfowl.h"
#include "vm_spec.h"
#include "vm-primitives.h"

#include <memory>

class VMPrimitiveInterface
{
  VirtualMachine& vm;

public:
  VMPrimitiveInterface(VirtualMachine& vm);

  void pop(std::size_t count);
  void push(oop value);
  void ret(oop value);
};

class VirtualMachine
{
  friend class VMPrimitiveInterface;

  struct ExecutionContext
  {
    int lexical_parent, sp;
    int ip, locals_begin;
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
  PrimitiveFunctionSet& primitives;

  int frame_ptr, frame_capacity;
  std::unique_ptr< ExecutionContext[] > frames;

  int stack_capacity;
  std::unique_ptr< oop[] > stack;

  vm_instruction_t* ip;
  oop* sp;
  object_array immediates;

public:
  enum LogLevel {
    LL_None, LL_Error, LL_Warn, LL_Info, LL_Debug, LL_Trace
  } log_level = LL_Warn;

protected:
  void enter_method(oop method);
  ExecutionContext* fp() const;
  void entered_frame();

  void execute_primitive(intmax_t index, int, oop*);

public:
  VirtualMachine(Image& image, PrimitiveFunctionSet& primitives, oop entrypoint_method, oop lobby_value = oop(0));
  bool lookup(oop receiver, oop selector, oop& result) const;
  void run(int ticks = 1024);

  void register_primitive(intmax_t index, PrimitiveFunction::function_t fn, const char* selector, void* dylib, const char* symbol_name);

};
