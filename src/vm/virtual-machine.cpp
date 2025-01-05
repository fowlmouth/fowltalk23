#include "virtual-machine.h"

VMPrimitiveInterface::VMPrimitiveInterface(VirtualMachine& vm)
: vm(vm)
{
}

void VMPrimitiveInterface::pop(std::size_t count)
{
  if(vm.log_level >= VirtualMachine::LL_Trace)
  {
    std::cerr << "[primitive pop count= " << count << ']' << std::endl;
  }
  vm.sp -= count;
}

void VMPrimitiveInterface::push(oop value)
{
  if(vm.log_level >= VirtualMachine::LL_Trace)
  {
    std::cerr << "[primitive push value= " << value << ']' << std::endl;
  }
  *vm.sp++ = value;
}


void VirtualMachine::enter_method(oop method)
{
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
  fr->sp = fr->locals_begin = sp - stack.get();

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

void VirtualMachine::execute_primitive(intmax_t index, int argc, oop* argv)
{
  if(auto prim = primitives.get((primitive_id_t)index))
  {
    if(prim->fn)
    {
      VMPrimitiveInterface iface(*this);
      int result = prim->fn(iface, argc, argv);
      (void)result;
    }
    else
    {
      std::cerr << "primitive not implemented index= " << index << std::endl;
    }
  }
  else
  {
    std::cerr << "invalid primitive index= " << index << std::endl;
  }
}

VirtualMachine::VirtualMachine(Image& image, PrimitiveFunctionSet& primitives, oop entrypoint_method)
: image(image),
  primitives(primitives),
  frame_ptr(0), frame_capacity(64),
  frames(std::make_unique< ExecutionContext[] >(64)),
  stack_capacity(128),
  stack(std::make_unique< oop[] >(128)),
  ip(nullptr), immediates(nullptr)
{
  sp = stack.get();
  auto fr = fp();
  fr->sp = fr->ip = fr->locals_begin = 0;
  fr->method = entrypoint_method;
  entered_frame();
}

bool VirtualMachine::lookup(oop receiver, oop selector, oop& result) const
{
  auto vt = oop_vtable(receiver, image);
  string_ref selector_sym = (string_ref)image.ptr(selector);
  vtable_slot_flags slot_flags = vts_invalid1;
  if(vt->lookup(image, image.ptr(receiver), selector_sym, &slot_flags, result))
  {
    return true;
  }

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
      if(log_level >= LL_Trace)
      {
        std::cerr << "[halt]" << std::endl;
      }
      return;

    case VMI_LoadImmediate:
      if(log_level >= LL_Trace)
      {
        std::cerr << "[load_immediate arg=" << arg << " ]" << std::endl;
      }
      *sp++ = immediates[arg];
      break;

    case VMI_Send:
    {
      if(log_level >= LL_Trace)
      {
        std::cerr << "[send arg=" << arg << " ]" << std::endl
          << "  selector= '" << (string_ref)image.ptr(*(sp-1)) << "'" << std::endl
          << "  receiver oop= " << *(sp-(1+arg)) << std::endl;
      }
      oop result;
      if(lookup(*(sp-(1+arg)), *(sp-1), result))
      {
        if(log_level >= LL_Debug)
        {
          std::cerr << "  found oop= " << result << std::endl;
        }
        vtable_object* vt = oop_vtable(result, image);
        oop bytecode = vt->bytecode();
        --sp; // pop the selector
        oop* argv = sp - arg;
        if(bytecode)
        {
          // activatable method
          if(log_level >= LL_Debug)
          {
            std::cerr << "  bytecode= " << bytecode << std::endl;
          }
          oop* bytecode_slots = (oop*)image.ptr(bytecode);
          if(bytecode_slots[VMBS_PrimitiveProxyIndex])
          {
            // alias method for a primitive
            if(log_level >= LL_Debug)
            {
              std::cerr << "  alias for primitive id= " << oop_to_int(bytecode_slots[VMBS_PrimitiveProxyIndex]) << std::endl;
            }
            execute_primitive(oop_to_int(bytecode_slots[VMBS_PrimitiveProxyIndex]), arg, argv);
          }
          else
          {
            enter_method(result);
          }
        }
        else
        {
          if(log_level >= LL_Debug)
          {
            std::cerr << "  not a method" << std::endl;
          }
        }
      }
      else
      {
        if(log_level >= LL_Debug)
        {
          std::cerr << "  not found selector='" << (string_ref)image.ptr(*(sp-1)) << '"' << std::endl;
        }
      }
      break;
    }

    case VMI_Return:
      if(log_level >= LL_Trace)
      {
        std::cerr << "[return arg=" << arg << " ]" << std::endl;
      }
      break;

    case VMI_Extend:
      if(log_level >= LL_Trace)
      {
        std::cerr << "[extend arg=" << arg << " ]" << std::endl;
      }
      continue;

    case VMI_SetLocal:
      if(log_level >= LL_Trace)
      {
        std::cerr << "[set_local arg=" << arg << " ]" << std::endl;
      }
      stack[fp()->locals_begin + arg] = *--sp;
      break;

    case VMI_LoadLocal:
      if(log_level >= LL_Trace)
      {
        std::cerr << "[load_local arg=" << arg << " ]" << std::endl;
      }
      *sp++ = stack[fp()->locals_begin + arg];
      break;

    default:
      if(log_level >= LL_Trace)
      {
        std::cerr << "[invalid opcode=" << op << " ]" << std::endl;
      }
      return;
    }
    arg = 0;
  }
}

void VirtualMachine::register_primitive(intmax_t index, PrimitiveFunction::function_t fn,
    const char* selector, void* dylib, const char* symbol_name)
{
  primitives.register_primitive(image, (primitive_id_t)index, fn, selector, dylib, symbol_name);
}
