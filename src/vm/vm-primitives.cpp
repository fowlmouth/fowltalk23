#include "vm-primitives.h"
#include "virtual-machine.h"

#define PRIM(name) \
  static int prim_##name(VMPrimitiveInterface& vm, int argc, oop* argv)

PRIM(Object_copy)
{
  (void)vm;
  (void)argc;
  (void)argv;
  return 0;
}

PRIM(Integer_plus_)
{
  (void)vm;
  (void)argc;
  (void)argv;
  if(argc != 2)
  {
    return -1;
  }
  oop a = argv[0], b = argv[1];
  vm.pop(2);
  if(!oop_is_int(a) || !oop_is_int(b))
  {
    vm.push(oop(0));
    return -1;
  }
  oop result = int_to_oop(oop_to_int(a) + oop_to_int(b));
  vm.push(result);
  return 0;
}

PRIM(Integer_print)
{
  (void)vm;
  (void)argc;
  (void)argv;
  if(argc != 1)
  {
    return -1;
  }
  oop a = argv[0];
  if(!oop_is_int(a))
  {
    return -1;
  }
  std::cout << oop_to_int(a) << std::endl;
  return 0;
}

void load_default_primitives(VirtualMachine& vm)
{
#define REG(Identifier, Selector) \
  vm.register_primitive(pid_##Identifier, prim_##Identifier, (Selector), nullptr, "prim_"#Identifier)

  REG(Object_copy, "_Object_copy");
  REG(Integer_plus_, "_Integer_plus:");
  REG(Integer_print, "_Integer_print");
}

PrimitiveFunctionSet::PrimitiveFunctionSet(int capacity)
: primitive_count(0), primitive_capacity(capacity)
{
  primitive_functions.reset(new PrimitiveFunction[ capacity ]);
}

void PrimitiveFunctionSet::register_primitive(Image& image, primitive_id_t pid, PrimitiveFunction::function_t fn,
  const char* selector, void* dylib, const char* symbol_name)
{
  long long index = (long long)pid;
  if(index < 0 || index > primitive_capacity)
  {
    // if(log_level >= LL_Error)
    //   std::cerr << "invalid primitive index= " << index << std::endl;
    throw TODO{};
  }
  if(primitive_functions[ index ].fn)
  {
    // if(log_level >= LL_Error)
    // {
    //   std::cerr << "primitive already registered index= " << index << std::endl;
    // }
    throw TODO{};
  }

  oop primitiveMap = image.special_object(soid_primitiveMap);
  vtable_object* primitiveMapVT = oop_vtable(primitiveMap, image);

  primitiveMapVT->add_slot(image, selector, vts_static, int_to_oop(index));
  primitive_functions[ index ] = {fn, dylib, symbol_name};
}

const PrimitiveFunction* PrimitiveFunctionSet::get(primitive_id_t pid) const
{
  long long index = (long long)pid;
  if(index < 0 || index > primitive_capacity)
  {
    // if(log_level >= LL_Error)
    //   std::cerr << "invalid primitive index= " << index << std::endl;
    throw TODO{};
  }
  return &primitive_functions[ index ];
}
