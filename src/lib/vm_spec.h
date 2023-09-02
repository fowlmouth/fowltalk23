#pragma once

#include <cstdint>

using vm_instruction_t = uint8_t;

enum VMInstruction
{
  VMI_Halt, VMI_LoadImmediate, VMI_Send, VMI_Return, VMI_Extend,

  VMI__count,
  VMI__bits = 2,
  VMI__mask = (1 << VMI__bits) - 1,
  VMI__argument_bits = CHAR_BIT * sizeof(vm_instruction_t) - VMI__bits,
  VMI__argument_mask = (1 << VMI__argument_bits) - 1
};

enum VMBytecodeSlots
{
  VMBS_Instructions, VMBS_Immediates,

  VMBS__count
};
