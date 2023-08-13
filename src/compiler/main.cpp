#include "libfowl.h"
#include "parser.h"
#include "cli.h"
#include "method-parser.h"

#include <iostream>

#include <cstdio>

// OSX
#include <sys/mman.h>
//  for lseek
#include <unistd.h>

#include <iostream>



class EventPrinter : public Parser
{
  std::ostream& stream;

public:
  EventPrinter(std::string_view input, std::ostream& stream);

protected:
  bool accept_integer(intmax_t value) override;
  bool accept_send(std::string_view selector, int arity) override;
};

EventPrinter::EventPrinter(std::string_view input, std::ostream& stream)
: Parser(input), stream(stream)
{
}

bool EventPrinter::accept_integer(intmax_t number)
{
  stream << "accept_integer number= " << number << std::endl;
  return true;
}

bool EventPrinter::accept_send(std::string_view selector, int arity)
{
  stream << "accept_send selector= '" << selector << "'  arity= " << arity << std::endl;
  return true;
}


int main(int argc, const char** argv)
{
  bool lexer_debug = false, parser_debug = false;
  std::string input_path;
  std::string_view input_contents;

  CLI cli;
  cli
    .on("--debug-lexer", [&](){
      lexer_debug = true;
    })
    .on("--debug-parser", [&](){
      parser_debug = true;
    })
    .on("--file", [&](std::string_view arg){
      input_path = arg;
    })
    .on("--input", [&](std::string_view arg){
      input_contents = arg;
    })
    .on_argument([&](std::string_view arg){
      input_path = arg;
    })
    .parse(argc, argv);

  void* mmap_data = nullptr;

  if(input_contents.empty() && !input_path.empty())
  {
    FILE* input_file = fopen(input_path.c_str(), "r");
    if(!input_file)
    {
      std::cerr << "failed to open file '" << input_path << '"' << std::endl;
      return 1;
    }
    int fileid = fileno(input_file);
    int file_size = lseek(fileid, 0, SEEK_END);
    fseek(input_file, 0, SEEK_SET);
    mmap_data = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fileid, 0);
    fclose(input_file);
    if(!mmap_data)
    {
      std::cerr << "failed to mmap file '" << input_path << '"' << std::endl;
      return 1;
    }
    input_contents = std::string_view((char*)mmap_data, file_size);
  }

  if(input_contents.empty())
  {
    std::cerr << "no input" << std::endl;
    return 1;
  }

  if(lexer_debug)
  {
    Lexer l(input_contents);

    for(auto tk = l.next(); tk.type != Token::EndOfFile; tk = l.next())
    {
      std::cout << "token type= " << tk.type << "  line= " << tk.source_line << "  column= " << tk.source_col << "  string= '" << tk.string << "'" << std::endl;
    }
  }

  if(parser_debug)
  {
    EventPrinter p(input_contents, std::cout);
    p.parse_expression();
  }

  MethodBuilder method_context;
  MethodParser parser(input_contents, method_context);
  parser.parse_expression();

  if(mmap_data)
  {
    munmap(mmap_data, input_contents.size());
  }

  return 0;
}
