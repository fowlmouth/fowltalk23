#include "libfowl.h"
#include "parser.h"
#include "cli.h"

#include <iostream>



int main(int argc, const char** argv)
{
  bool lexer_debug = false;

  CLI cli;

  cli
    .on("--debug-lexer", [&lexer_debug](){ lexer_debug = true; })
    .parse(argc, argv);

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
