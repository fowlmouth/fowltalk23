#include "method-parser.h"

MethodParser::MethodParser(MethodBuilder& builder)
: builder_(&builder)
{
}

bool MethodParser::accept_integer(intmax_t number)
{
  builder_->load_immediate_integer(number);
  return true;
}

bool MethodParser::accept_identifier(std::string_view name)
{
  std::string string_name = std::string(name);
  int index = builder_->lookup_variable(string_name);
  if(index == -1)
  {
    // local not found, send a message
    builder_->send_message(name, 0);
  }
  else
  {
    builder_->load_local(index);
  }
  return true;
}

bool MethodParser::accept_send(std::string_view selector, int arity)
{
  builder_->send_message(selector, arity);
  return true;
}

bool MethodParser::accept_assignment(std::string_view name)
{
  std::string string_name = std::string(name);
  int index = builder_->declare_variable(string_name);
  if(index == -1)
  {
    index = builder_->lookup_variable(string_name);
    builder_->set_local(index);
  }
  return true;
}
