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
    .parse(argc, argv);

  Memory mem;

  Image im(mem);
  if(!image_path.empty())
  {
    im.load(image_path.c_str());
  }
  else
  {
    // ???
    im.bootstrap();
  }

  return 0;
}
