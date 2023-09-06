#include "virtual-machine.h"

void VirtualMachine::enter_method(oop method)
{
  (void)method;
  // write locals to the current frame
  auto fr = fp();
  object_array bytecode_slots = fr->bytecode_slots(image);
  fr->ip = ip - (vm_instruction_t*)image.ptr(bytecode_slots[VMBS_Instructions]);
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
  object_array bytecode_slots = fr->bytecode_slots(image);
  ip = (vm_instruction_t*)image.ptr(bytecode_slots[VMBS_Instructions]) + fr->ip;
  sp = stack.get() + fr->sp;
  immediates = (object_array)image.ptr(bytecode_slots[VMBS_Immediates]);
}

VirtualMachine::VirtualMachine(Image& image, oop entrypoint_method)
: image(image),
  frame_ptr(0), frame_capacity(64),
  frames(std::make_unique< ExecutionContext[] >(64)),
  stack_capacity(128),
  stack(std::make_unique< oop[] >(128)),
  ip(nullptr), immediates(nullptr)
{
  sp = stack.get();
  auto fr = fp();
  fr->sp = fr->ip = 0;
  fr->method = entrypoint_method;
  entered_frame();
}

bool VirtualMachine::lookup(oop receiver, oop selector, oop& result) const
{
  auto vt = oop_vtable(receiver, image);
  (void)vt;

  (void)selector;
  (void)result;

  result = 0;

  return false;
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
      *sp++ = immediates[arg];
      break;

    case VMI_Send:
    {
      std::cout << "[send arg=" << arg << " ]" << std::endl;
      std::cout << "  selector= '" << (string_ref)image.ptr(*(sp-1)) << "'" << std::endl;
      std::cout << "  receiver oop= " << *(sp-(1+arg)) << std::endl;
      oop result;
      if(lookup(*(sp-(1+arg)), *(sp-1), result))
      {
        std::cout << "  found oop= " << result << std::endl;
      }
      else
      {
        std::cout << "  not found" << std::endl;
      }
      break;
    }

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
