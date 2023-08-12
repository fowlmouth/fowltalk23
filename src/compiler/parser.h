#pragma once

#include "lexer.h"

class Parser
{
  Lexer lexer;
  Token tok;

  void next();

  Token::Type current_type() const;

protected:
  virtual bool accept_integer(intmax_t value) = 0;

public:
  Parser(std::string_view input);
  virtual ~Parser();

  void parse_expression();

};
