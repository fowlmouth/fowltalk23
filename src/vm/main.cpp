#include "libfowl.h"


int main()
{
  Memory mem;

  Image im(mem);
  im.bootstrap();

  return 0;
}
