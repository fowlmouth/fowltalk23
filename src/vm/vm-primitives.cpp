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

#define INT_CHECK \
  if(argc != 2) \
  { \
    return -1; \
  } \
  oop a = argv[0], b = argv[1]; \
  if(!oop_is_int(a) || !oop_is_int(b)) \
  { \
    vm.pop(1); \
    return -1; \
  }

PRIM(Integer_plus_)
{
  INT_CHECK;
  oop result = int_to_oop(oop_to_int(a) + oop_to_int(b));
  vm.pop(2);
  vm.push(result);
  return 0;
}

PRIM(Integer_minus_)
{
  INT_CHECK;
  oop result = int_to_oop(oop_to_int(a) - oop_to_int(b));
  vm.pop(2);
  vm.push(result);
  return 0;
}

PRIM(Integer_multiply_)
{
  INT_CHECK;
  oop result = int_to_oop(oop_to_int(a) * oop_to_int(b));
  vm.pop(2);
  vm.push(result);
  return 0;
}

PRIM(Integer_divide_)
{
  INT_CHECK;
  oop result = int_to_oop(oop_to_int(a) / oop_to_int(b));
  vm.pop(2);
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

void PrimitiveFunctionSet::load_defaults(Image& image)
{
#define REG(Identifier, Selector) \
  register_primitive(image, pid_##Identifier, prim_##Identifier, (Selector), nullptr, "prim_"#Identifier)

  REG(Object_copy, "_Object_copy");
  REG(Integer_plus_, "_Integer_plus:");
  REG(Integer_minus_, "_Integer_minus:");
  REG(Integer_multiply_, "_Integer_multiply:");
  REG(Integer_divide_, "_Integer_divide:");
  REG(Integer_print, "_Integer_print");
}
