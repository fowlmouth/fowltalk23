#include "utest.h"
#include "virtual-machine.h"

UTEST(vm, init) {
  Image image(4096);
  image.bootstrap();
  PrimitiveFunctionSet primitives(32);
  primitives.load_defaults(image);
  oop entrypoint_method = 0;
  VirtualMachine vm(image, primitives, entrypoint_method);
}
