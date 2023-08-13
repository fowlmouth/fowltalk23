#include "method-builder.h"

MethodBuilder::MethodBuilder(Image& image)
: parent_(nullptr), image_(&image), stack_size(0), stack_size_max(0)
{
}

MethodBuilder::MethodBuilder(MethodBuilder* parent)
: parent_(parent), image_(nullptr), stack_size(0), stack_size_max(0)
{
  if(parent_)
    image_ = parent_->image_;
}

int MethodBuilder::push_immediate(oop value)
{
  immediates.push_back(value);
  return immediates.size() - 1;
}

int MethodBuilder::immediate_unique_push(oop value)
{
  const auto iter = std::find(immediates.begin(), immediates.end(), value);
  if(iter == immediates.end())
  {
    return push_immediate(value);
  }
  else
  {
    return std::distance(immediates.begin(), iter);
  }
}

int MethodBuilder::intern(const char* symbol)
{
  oop val = image_->intern(symbol);
  int index = immediate_unique_push(val);
  return index;
}

void MethodBuilder::write_instruction(VMInstruction instruction, intmax_t arg)
{
  switch(instruction)
  {
  case VMI_Halt:
    break;
  case VMI_LoadImmediate:
    ++stack_size;
    break;
  case VMI_Send:
    --stack_size; // for the selector
    stack_size -= arg;
    break;

  default:
    break;
  }
  if(stack_size > stack_size_max)
  {
    stack_size_max = stack_size;
  }

  vm_instruction_t op = (vm_instruction_t)instruction & VMI__mask;
  op |= (arg & VMI__argument_mask) << VMI__bits;
  instructions.push_back(op);
}

void MethodBuilder::load_immediate_integer(intmax_t value)
{
  oop val = int_to_oop(value);
  int index;
  {
    const auto iter = std::find(immediates.begin(), immediates.end(), val);
    if(iter == immediates.end())
    {
      index = push_immediate(val);
    }
    else
    {
      index = std::distance(immediates.begin(), iter);
    }
  }
  write_instruction(VMI_LoadImmediate, index);
}

void MethodBuilder::send_message(const char* selector, int arg_count)
{
  int selector_index = intern(selector);
  write_instruction(VMI_LoadImmediate, selector_index);
  write_instruction(VMI_Send, arg_count);
}
