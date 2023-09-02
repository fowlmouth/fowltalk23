#include "virtual-machine.h"

void VirtualMachine::enter_method(oop method)
{
  (void)method;
  // write locals to the current frame
  auto fr = fp();
  object_array bytecode_slots = fr->bytecode_slots(mem);
  fr->ip = ip - (vm_instruction_t*)mem.ptr(bytecode_slots[VMBS_Instructions]);
  fr->sp = sp - stack.get();

  // push new frame
  if(frame_ptr == frame_capacity)
  {
    std::cerr << "call stack overflow" << std::endl;
    throw TODO{};
  }
  ++frame_ptr;
  fr = fp();
  fr->method = method;
  fr->ip = 0;
  fr->sp = sp - stack.get();

  entered_frame();
}

VirtualMachine::ExecutionContext* VirtualMachine::fp() const
{
  return frames.get() + frame_ptr;
}

void VirtualMachine::entered_frame()
{
  auto fr = fp();
  object_array bytecode_slots = fr->bytecode_slots(mem);
  ip = (vm_instruction_t*)mem.ptr(bytecode_slots[VMBS_Instructions]) + fr->ip;
  sp = stack.get() + fr->sp;
  immediates = (object_array)mem.ptr(bytecode_slots[VMBS_Immediates]);
}

VirtualMachine::VirtualMachine(Memory& mem, oop entrypoint_method)
: mem(mem),
  frame_ptr(0), frame_capacity(64),
  frames(std::make_unique< ExecutionContext[] >(64)),
  stack_capacity(128),
  stack(std::make_unique< oop[] >(128)),
  ip(nullptr)
{
  auto fr = fp();
  fr->sp = fr->ip = 0;
  fr->method = entrypoint_method;
  entered_frame();
}

void VirtualMachine::run(int ticks)
{
  VMInstruction op = VMI_Halt;
  intmax_t arg = 0;

  while(ticks-- > 0 || op == VMI_Extend)
  {
    vm_instruction_t instr = *ip++;
    op = (VMInstruction)(instr & VMI__mask);
    arg = (arg << VMI__argument_bits) | (instr >> VMI__bits);
    switch(op)
    {
    case VMI_Halt:
      std::cout << "[halt]" << std::endl;
      return;

    case VMI_LoadImmediate:
      std::cout << "[load_immediate arg=" << arg << " ]" << std::endl;
      break;

    case VMI_Send:
      std::cout << "[send arg=" << arg << " ]" << std::endl;
      break;

    case VMI_Return:
      std::cout << "[return arg=" << arg << " ]" << std::endl;
      break;

    case VMI_Extend:
      std::cout << "[extend arg=" << arg << " ]" << std::endl;
      continue;

    default:
      std::cout << "[invalid opcode=" << op << " ]" << std::endl;
      return;
    }
    arg = 0;
  }
}
