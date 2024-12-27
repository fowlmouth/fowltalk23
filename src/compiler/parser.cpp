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

bool Parser::current_type(Token::Type type) const
{
  return tok.type == type;
}

void Parser::report_error(std::string message)
{
  std::cerr << "Parser error '" << message
    << "' line= " << tok.source_line
    << " column= " << tok.source_col << std::endl;
}

Token::Position Parser::save_position() const
{
  return {tok.source_index, tok.source_line, tok.source_col};
}

void Parser::restore_position(Token::Position position)
{
  lexer.reset_position(position.source_index, position.source_line, position.source_col);
  next();
}

#define CHECK_CALLBACK(callback) \
  do{ \
    if(!(callback)) \
    { \
      return false; \
    } \
  }while(0)

#define EXPECT(parse_fn, name_string) \
  if(!(parse_fn())) \
  { \
    report_error(std::string("expected: ") + std::string(name_string)); \
    return false; \
  }

#define EXPECT_TOKEN(token_type) \
  if(current_type() != token_type) \
  { \
    report_error(std::string("expected: ") + token_type_to_string(token_type)); \
    return false; \
  } \
  next();

bool Parser::parse_terminal()
{
  switch(current_type())
  {
  case Token::Integer:
    CHECK_CALLBACK(accept_integer(tok.int_value));
    next();
    return true;

  case Token::OpenParen:
    next();
    EXPECT(parse_expression, "expression");
    EXPECT_TOKEN(Token::CloseParen);
    return true;

  default:
    break;
  }
  return false;
}

bool Parser::parse_unary()
{
  if(parse_terminal())
  {
    while(current_type() == Token::Identifier)
    {
      CHECK_CALLBACK(accept_send(tok.string, 1));
      next();
    }
    return true;
  }
  return false;
}

bool Parser::parse_infix()
{
  if(parse_unary())
  {
    if(current_type() == Token::Operator)
    {
      Token op = tok;
      next();
      EXPECT(parse_unary, "unary expression");
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
  if(current_type(Token::Identifier))
  {
    auto position = save_position();
    auto dest = tok;
    next();
    if(current_type(Token::Assignment))
    {
      next();
      EXPECT(parse_expression, "expression");
      CHECK_CALLBACK(accept_assignment(dest.string));
      return true;
    }
    restore_position(position);
  }
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
