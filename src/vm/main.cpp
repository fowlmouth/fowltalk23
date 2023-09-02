#include "libfowl.h"
#include "cli.h"

#include "virtual-machine.h"

#include <string>

int main(int argc, const char** argv)
{
  std::string image_path;

  CLI cli;
  cli
    .on("--image", [&](std::string_view value){
      image_path = value;
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
  std::cout << "entrypoint= " << entrypoint << std::endl;

  VirtualMachine vm(im, entrypoint);
  vm.run();

  return 0;
}
