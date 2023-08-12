#include "parser.h"

Parser::Parser(std::string_view input)
: lexer(input)
{
  next();
}

Parser::~Parser()
{
}

void Parser::next()
{
  tok = lexer.next();
}

Token::Type Parser::current_type() const
{
  return tok.type;
}

void Parser::parse_expression()
{
  switch(current_type())
  {
  case Token::tk_integer:
    accept_integer(tok.int_value);
    next();
    break;

  default:
    break;
  }
}
