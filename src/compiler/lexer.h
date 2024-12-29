#pragma once

#include <string_view>

struct Token
{
  enum Type
  {
    EndOfFile,
    Identifier,
    Operator,
    Integer,
    String,
    Assignment,
    Period,
    OpenParen,
    CloseParen,

    __count
  };

  struct Position
  {
    std::size_t source_index, source_line, source_col;
  };

  Type type;
  std::string_view string;
  std::size_t source_index, source_line, source_col;
  intptr_t int_value;

  Token();
  Token(Type type);
};

const char* token_type_to_string(Token::Type type);

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
  void reset_position(std::size_t index, std::size_t line, std::size_t col);
};
