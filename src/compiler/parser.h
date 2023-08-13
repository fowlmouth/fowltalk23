#pragma once

#include "lexer.h"

#include <iostream>

class Parser
{
  Lexer lexer;
  Token tok;

  void next();

  Token::Type current_type() const;

protected:
  virtual bool accept_integer(intmax_t value) = 0;

  virtual bool accept_send(std::string_view selector, int arity) = 0;

  virtual void report_error(std::string message);

public:
  Parser(std::string_view input);
  virtual ~Parser();

  bool parse_terminal();

  bool parse_infix();

  bool parse_expression();

};
