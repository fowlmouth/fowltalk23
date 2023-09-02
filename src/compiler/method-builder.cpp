#include "method-builder.h"

MethodBuilder::MethodBuilder(Image& image)
: parent_(nullptr), image_(&image), stack_size(0), stack_size_max(0)
{
}

MethodBuilder::MethodBuilder(MethodBuilder* parent)
: parent_(parent), image_(nullptr), stack_size(0), stack_size_max(0)
{
  if(parent_)
  {
    image_ = parent_->image_;
  }
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
  oop val = image_->offset(image_->intern(symbol));
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
    // --stack_size; // for the selector
    stack_size -= arg;
    // ++stack_size; // for the result
    // ^ commented out because they cancel each other out
    break;

  default:
    break;
  }
  if(stack_size > stack_size_max)
  {
    stack_size_max = stack_size;
  }
  if(stack_size < 0)
  {
    throw MethodBuilder::StackUnderflowError{};
  }

  vm_instruction_t buffer[16];

  vm_instruction_t *op = buffer + 16;

  *--op = ((vm_instruction_t)instruction & VMI__mask)
    | ((vm_instruction_t)(arg & VMI__argument_mask) << VMI__bits);
  arg >>= VMI__argument_bits;
  while(arg && op > buffer)
  {
    *--op = (vm_instruction_t)VMI_Extend
      | ((vm_instruction_t)(arg & VMI__argument_mask) << VMI__bits);
    arg >>= VMI__argument_bits;
  }
  instructions.insert(instructions.end(), op, buffer+16);
}

void MethodBuilder::load_immediate_integer(intmax_t value)
{
  oop val = int_to_oop(value);
  int index = immediate_unique_push(val);
  write_instruction(VMI_LoadImmediate, index);
}

void MethodBuilder::send_message(std::string_view selector, int arg_count)
{
  auto selector_copy = std::make_unique< char[] >(selector.size() + 1);
  std::memcpy(selector_copy.get(), selector.data(), selector.size());
  selector_copy[selector.size()] = 0;
  int selector_index = intern(selector_copy.get());
  write_instruction(VMI_LoadImmediate, selector_index);
  write_instruction(VMI_Send, arg_count);
}

oop MethodBuilder::as_method() const
{
  // build literals
  object_array literals = (object_array)image_->alloc_words((vtable_object*)image_->ptr(image_->special_object(soid_arrayVt)), immediates.size());
  std::memcpy(literals, immediates.data(), immediates.size() * sizeof(oop));

  // build instructions
  vm_instruction_t* instr = (vm_instruction_t*)image_->alloc((vtable_object*)image_->ptr(image_->special_object(soid_symbolVt)), instructions.size());
  std::memcpy(instr, instructions.data(), instructions.size());

  // build bytecode
  object_array bytecode = (object_array)image_->alloc_words((vtable_object*)image_->ptr(image_->special_object(soid_arrayVt)), VMBS__count);
  bytecode[VMBS_Instructions] = image_->offset(instr);
  bytecode[VMBS_Immediates] = image_->offset(literals);

  // build vtable
  const int slot_count = arity;
  auto vtable_vt = (vtable_object*)image_->ptr(image_->special_object(soid_vtableVt));
  auto vt = vtable_object::make(slot_count, 0, vtable_vt, *image_);
  for(int i = 0; i < arity; ++i)
  {
    vt->add_slot(*image_, variable_names.at(i).c_str(), vts_data, 0);
  }
  vt->set_bytecode(image_->offset(bytecode));

  // instantiate method
  auto method = image_->alloc_words(vt, slot_count);

  return image_->offset(method);
}