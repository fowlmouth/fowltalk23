#pragma once

enum primitive_id_t
{
  pid_Object_copy, pid_Object_vtable, pid_Object_vtable_,
  pid_VTable_allocateBytes_flags_,

  pid_Integer_plus_, pid_Integer_minus_,
  pid_Integer_multiply_, pid_Integer_divide_,
  pid_Integer_print,

  pid__count
};

enum special_objects_index_t
{
  soid_vtableVt, soid_symbolVt, soid_integerVt, soid_primitiveMap, soid_symbolMapVt, soid_lobby, soid_arrayVt,

  soid__count
};
