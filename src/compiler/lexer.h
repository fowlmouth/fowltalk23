#pragma once

#include <string_view>

struct Token
{
  enum Type
  {
    tk_eof,
    tk_identifier,
    tk_integer,
    tk_string,

    tk__count
  };

  Type type;
  std::string_view string;
  std::size_t source_index, source_line, source_col;
  intptr_t int_value;
};

class Lexer
{
  std::string_view input;
  std::size_t index, line, col;

  char current() const;

  bool is_whitespace() const;
  bool is_digit() const;
  bool is_identifier_start() const;
  bool is_identifier_char() const;

  void next_char();

public:
  Lexer(std::string_view input);

  Token next();
};
