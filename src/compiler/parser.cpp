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

#define CHECK_CALLBACK(callback) \
  do{ \
    if(!(callback)) \
    { \
      return false; \
    } \
  }while(0)


bool Parser::parse_terminal()
{
  switch(current_type())
  {
  case Token::Integer:
    CHECK_CALLBACK(accept_integer(tok.int_value));
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
    if(current_type() == Token::Operator)
    {
      Token op = tok;
      next();
      EXPECT(parse_terminal, "terminal");
      CHECK_CALLBACK(accept_send(op.string, 2));
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

bool Parser::parse_method_definition()
{
  return false;
}

bool Parser::parse_assignment()
{
  return false;
}

bool Parser::parse_statement()
{
  return parse_method_definition() || parse_assignment() || parse_expression();
}

bool Parser::parse_document()
{
  bool res = false;
  while(parse_statement())
  {
    res = true;
  }
  return res;
}
