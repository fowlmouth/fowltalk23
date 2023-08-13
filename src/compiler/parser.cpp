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

void Parser::report_error(std::string message)
{
  std::cerr << "Parser error '" << message
    << "' line= " << tok.source_line
    << " column= " << tok.source_col << std::endl;
}

bool Parser::parse_terminal()
{
  switch(current_type())
  {
  case Token::tk_integer:
    accept_integer(tok.int_value);
    next();
    return true;

  default:
    break;
  }
  return false;
}

#define EXPECT(parse_fn, name_string) \
  if(!(parse_fn())) \
  { \
    report_error(std::string("expected: ") + std::string(name_string)); \
    return false; \
  }

bool Parser::parse_infix()
{
  if(parse_terminal())
  {
    if(current_type() == Token::tk_operator)
    {
      Token op = tok;
      next();
      EXPECT(parse_terminal, "terminal");
      accept_send(op.string, 2);
      return true;
    }
    return true;
  }
  return false;
}

bool Parser::parse_expression()
{
  return parse_infix();
}
