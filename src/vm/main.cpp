#include "libfowl.h"
#include "cli.h"

#include "virtual-machine.h"
#include "vm-primitives.h"

#include <string>

int main(int argc, const char** argv)
{
  std::string image_path;
  VirtualMachine::LogLevel log_level = VirtualMachine::LL_Warn;

  CLI cli;
  cli
    .on("--image", [&](std::string_view value){
      image_path = value;
    })
    .on("--log-level", [&](std::string_view value){
      if(!strncasecmp("error", value.data(), 5))
        log_level = VirtualMachine::LL_Error;
      else if(!strncasecmp("warn", value.data(), 4))
        log_level = VirtualMachine::LL_Warn;
      else if(!strncasecmp("info", value.data(), 4))
        log_level = VirtualMachine::LL_Info;
      else if(!strncasecmp("debug", value.data(), 5))
        log_level = VirtualMachine::LL_Debug;
      else if(!strncasecmp("trace", value.data(), 5))
        log_level = VirtualMachine::LL_Trace;
      else
      {
        std::cerr << "Valid log levels: error, warn, info, debug, trace" << std::endl;
        log_level = VirtualMachine::LL_Debug;
      }
    })
    .on("--help", [&](){
      std::cout << "usage: " << argv[0] << " [options] [--] [image]" << std::endl
        << "  --image [path]" << std::endl
        << "  --log-level [level]" << std::endl
        << "  --help" << std::endl;
    })
    .on_argument([&](std::string_view value){
      image_path = value;
    })
    .parse(argc, argv);

  Image im(image_path.c_str());

  oop entrypoint = 0;
  im.each_entrypoint([](oop entrypoint, void* arg_){
    oop* arg = (oop*)arg_;
    *arg = entrypoint;
    return false;
  }, &entrypoint);
  if(log_level >= VirtualMachine::LL_Debug)
  {
    std::cerr << "entrypoint= " << entrypoint << std::endl;
  }

  PrimitiveFunctionSet primitives(32);
  VirtualMachine vm(im, primitives, entrypoint);
  vm.log_level = log_level;
  load_default_primitives(vm);
  vm.run();

  return 0;
}
