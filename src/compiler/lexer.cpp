#include "lexer.h"

Lexer::Lexer(std::string_view input)
: input(input)
, index(0), line(0), col(0)
{
}

char Lexer::current() const
{
  return input[index];
}

bool Lexer::is_whitespace() const
{
  auto c = current();
  return c == ' ' || c == '\t' || c == '\n';
}

bool Lexer::is_digit() const
{
  return current() >= '0' && current() <= '9';
}

bool Lexer::is_identifier_start() const
{
  char c = current();
  return (c >= 'a' && c <= 'z') ||
    (c >= 'A' && c <= 'Z') ||
    c == '_';
}

bool Lexer::is_identifier_char() const
{
  return is_identifier_start() || is_digit();
}


void Lexer::next_char()
{
  switch(input[index++])
  {
  case 0:
    --index;
    break;
  case '\n':
    ++line;
    col = 0;
    break;
  default:
    ++col;
    break;
  }
}

Token Lexer::next()
{
  if(index >= input.size())
  {
    return Token{ .type = Token::EndOfFile };
  }

  while(is_whitespace())
  {
    next_char();
  }

  Token token;
  token.source_index = index;
  token.source_line = line;
  token.source_col = col;

  if(index >= input.size())
  {
    token.type = Token::EndOfFile;
    return token;
  }

  switch(current())
  {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    token.type = Token::Integer;
    token.int_value = 0;
    while(is_digit())
    {
      token.int_value = token.int_value * 10 + (current() - '0');
      next_char();
    }
    break;

  case '+': case '-': case '/': case '*':
    token.type = Token::Operator;
    next_char();
    break;

  default:
    if(is_identifier_start())
    {
      token.type = Token::Identifier;
      while(is_identifier_char())
      {
        next_char();
      }
    }
    else
    {
      token.type = Token::EndOfFile;
    }
    break;
  }

  token.string = input.substr(token.source_index, index - token.source_index);
  return token;
}


