#pragma once

#include "lexer.h"

#include <stack>
#include <iostream>

class ParserActions;

class ParserInterface
{
public:
  virtual ~ParserInterface();

  virtual void report_error(std::string message) = 0;

  virtual void push_actions(std::unique_ptr<ParserActions>) = 0;
  virtual void pop_actions() = 0;
};

class ParserActions
{
public:
  ParserInterface* interface;

  virtual ~ParserActions();

  virtual bool accept_integer(intmax_t value) = 0;
  virtual bool accept_identifier(std::string_view name) = 0;
  virtual bool accept_assignment(std::string_view name) = 0;
  virtual bool accept_send(std::string_view selector, int arity) = 0;
};

class Parser : public ParserInterface
{
  Lexer lexer;
  Token tok;

  std::stack< std::unique_ptr< ParserActions >> actions;

  void next();

  Token::Type current_type() const;
  bool current_type(Token::Type type) const;

protected:
  Token::Position save_position() const;
  void restore_position(Token::Position position);

  void report_error(std::string message) override;

public:
  Parser(std::string_view input, std::unique_ptr<ParserActions> first_actions);

  void push_actions(std::unique_ptr<ParserActions> actions) override;
  void pop_actions() override;

  ParserActions* top_action() const;

  bool parse_terminal();
  bool parse_unary();
  bool parse_infix();
  bool parse_expression();
  bool parse_method_definition();
  bool parse_assignment();
  bool parse_statement();
  bool parse_document();

};
