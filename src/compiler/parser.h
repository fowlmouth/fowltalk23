#pragma once

#include "lexer.h"

#include <iostream>

class Parser
{
  Lexer lexer;
  Token tok;

  void next();

  Token::Type current_type() const;
  bool current_type(Token::Type type) const;

protected:

  // 42
  virtual bool accept_integer(intmax_t value) = 0;

  // foo := 42
  virtual bool accept_assignment(std::string_view name) = 0;

  // foo bar: baz
  virtual bool accept_send(std::string_view selector, int arity) = 0;

  virtual void report_error(std::string message);

  Token::Position save_position() const;
  void restore_position(Token::Position position);

public:
  Parser(std::string_view input);
  virtual ~Parser();

  bool parse_terminal();
  bool parse_unary();
  bool parse_infix();
  bool parse_expression();
  bool parse_method_definition();
  bool parse_assignment();
  bool parse_statement();
  bool parse_document();

};
