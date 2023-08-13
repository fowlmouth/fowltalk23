#include "method-parser.h"

MethodParser::MethodParser(std::string_view input, MethodBuilder& builder)
: Parser(input), builder_(&builder)
{
}

bool MethodParser::accept_integer(intmax_t number)
{
  (void)number;
  return true;
}

bool MethodParser::accept_send(std::string_view selector, int arity)
{
  (void)selector;
  (void)arity;
  return true;
}
