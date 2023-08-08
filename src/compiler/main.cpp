#include "libfowl.h"

#include "parser.h"

#include <iostream>

int main()
{
  std::string my_input = "hello world";

  Lexer l(my_input);

  for(auto tk = l.next(); tk.type != Token::tk_eof; tk = l.next())
  {
    std::cout << "token type= " << tk.type << "  string= '" << tk.string << "'" << std::endl;
  }
  return 0;
}
