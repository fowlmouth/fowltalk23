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
  bool accept_assignment(std::string_view name) override;
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

bool EventPrinter::accept_assignment(std::string_view name)
{
  stream << "accept_assignment name= '" << name << "'" << std::endl;
  return true;
}



std::size_t interpret_image_size(const std::string& str, std::size_t default_value)
{
  if(str.empty())
  {
    return default_value;
  }
  std::size_t num = default_value;
  try
  {
    num = std::stol(str);
    switch(str.back())
    {
    case 'g': case 'G':
      num *= 1024 * 1024 * 1024;
      break;

    case 'm': case 'M':
      num *= 1024 * 1024;
      break;

    case 'k': case 'K':
      num *= 1024;
      break;

    case 'b':
    default:
      break;
    }
    return num;
  }
  catch(const std::invalid_argument& )
  {
    std::cerr << "invalid size value '" << str << "'" << std::endl;
  }
  catch(const std::out_of_range& )
  {
    std::cerr << "size value '" << str << "' is out of range" << std::endl;
  }
  return default_value;
}

int main(int argc, const char** argv)
{
  bool lexer_debug = false, parser_debug = false,
    compiler_debug = false;
  std::string input_path, output_path;
  std::string_view input_contents;
  std::string image_size = "1m";

  CLI cli;
  cli
    .on("--debug-lexer", [&](){
      lexer_debug = true;
    })
    .on("--debug-parser", [&](){
      parser_debug = true;
    })
    .on("--debug-compiler", [&](){
      compiler_debug = true;
    })
    .on("--file", [&](std::string_view arg){
      input_path = arg;
    })
    .on("--input", [&](std::string_view arg){
      input_contents = arg;
    })
    .on("--output", [&](std::string_view arg){
      output_path = arg;
    })
    .on("--size", [&](std::string_view arg){
      image_size = arg;
    })
    .on("--help", [&argv](){
      std::cout << "usage: " << argv[0] << " [options] [--] [file]" << std::endl
        << "  --debug-lexer" << std::endl
        << "  --debug-parser" << std::endl
        << "  --debug-compiler" << std::endl
        << "  --file [path]" << std::endl
        << "  --input [string]" << std::endl
        << "  --output [path]" << std::endl
        << "  --size [size]" << std::endl
        << "  --help" << std::endl;
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
    if(mmap_data == MAP_FAILED)
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
    p.parse_document();
  }

  std::size_t image_size_num = interpret_image_size(image_size, 1 << 20);
  Image image(image_size_num);
  image.bootstrap();

  MethodBuilder method_context(image);
  method_context.add_argument("lobby");
  MethodParser parser(input_contents, method_context);
  parser.parse_document();

  if(compiler_debug)
  {
    std::cout << "stack size max= " << method_context.stack_size_max << std::endl
      << "immediates (" << method_context.immediates.size() << ")" << std::endl;
    for(auto imm : method_context.immediates)
    {
      if(oop_is_int(imm))
      {
        std::cout << "  int= " << oop_to_int(imm) << std::endl;
      }
      else
      {
        std::cout << "  oop= " << imm << std::endl;
      }
    }
    std::cout << "instructions (" << method_context.instructions.size() << ")" << std::endl;
    for(auto instr : method_context.instructions)
    {
      VMInstruction op = (VMInstruction)(instr & VMI__mask);
      std::cout << "  op= " << op << std::endl;
    }
  }

  oop method = method_context.as_method();
  if(method)
  {
    image.add_entrypoint(method);
  }

  if(mmap_data)
  {
    munmap(mmap_data, input_contents.size());
  }

  if(!output_path.empty())
  {
    image.save(output_path.c_str());
  }

  return 0;
}
