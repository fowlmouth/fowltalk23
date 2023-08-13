#pragma once

#include "parser.h"

#include "object.h"
#include "vm_spec.h"
#include "libfowl.h"

#include <vector>
#include <iterator>

int main(int, const char**);

class MethodBuilder
{
  MethodBuilder* parent_;
  Image* image_;
  int stack_size, stack_size_max;
  std::vector< oop > immediates;
  std::vector< vm_instruction_t > instructions;

  friend int main(int, const char**);

protected:
  int push_immediate(oop value);
  int immediate_unique_push(oop value); // terrible name
  int intern(const char* selector);

  void write_instruction(VMInstruction instruction, intmax_t arg);

public:
  MethodBuilder(Image& image);
  MethodBuilder(MethodBuilder* parent);

  void load_immediate_integer(intmax_t value);
  void send_message(const char* selector, int arg_count);

};

