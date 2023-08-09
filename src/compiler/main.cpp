#include "libfowl.h"

#include "parser.h"

#include <iostream>



int main(int argc, const char** argv)
{
  bool lexer_debug = false;

  for(int i = 1; i < argc; ++i)
  {
    if(!std::strcmp("--debug-lexer", argv[i]))
    {
      lexer_debug = true;
    }
  }

  if(lexer_debug)
  {
    std::string my_input = "hello world";

    Lexer l(my_input);

    for(auto tk = l.next(); tk.type != Token::tk_eof; tk = l.next())
    {
      std::cout << "token type= " << tk.type << "  string= '" << tk.string << "'" << std::endl;
    }
  }

  return 0;
}
