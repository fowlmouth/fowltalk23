#pragma once

#include "lexer.h"

class Parser
{
  Lexer lexer;
public:
  Parser(std::string_view input);

};
