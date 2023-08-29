#include "libfowl.h"
#include "cli.h"

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

  try
  {
    Image im(image_path.c_str());

  oop entrypoint = 0;
  im.each_entrypoint([](oop entrypoint, void* arg_){
    oop* arg = (oop*)arg_;
    *arg = entrypoint;
    return false;
  }, &entrypoint);
  std::cout << "entrypoint= " << entrypoint << std::endl;


  }
  catch(const std::runtime_error& err)
  {
    std::cerr << "strerr: " << strerror(errno) << std::endl;
    switch(errno)
    {
    case EACCES:
      std::cerr << "Permission denied" << std::endl;
      break;
    case EBADF:
      std::cerr << "Bad file descriptor" << std::endl;
      break;
    case EINVAL:
      std::cerr << "Invalid argument" << std::endl;
      break;
    default:
      std::cerr << "other error " << errno << std::endl;
      break;
    }
  }

  return 0;
}
