#include "vm-primitives.h"
#include "virtual-machine.h"

int prim_Object_copy(VirtualMachine& vm, int, oop* argv)
{
  (void)vm;
  (void)argv;
  return 0;
}

int prim_Integer_plus_(VirtualMachine& vm, int, oop* argv)
{
  (void)vm;
  (void)argv;
  return 0;
}

void load_default_primitives(VirtualMachine& vm)
{
#define PRIM(Identifier, Selector) \
  vm.register_primitive(pid_##Identifier, prim_##Identifier, (Selector), nullptr, "prim_"#Identifier)

  PRIM(Object_copy, "_Object_copy");
  PRIM(Integer_plus_, "_Integer_plus:");
}
