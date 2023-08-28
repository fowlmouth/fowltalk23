#pragma once

enum primitive_id_t
{
  pid_Object_copy, pid_Object_vtable, pid_Object_vtable_,
  pid_VTable_allocateBytes_flags_,

  pid__count
};

enum special_objects_index_t
{
  soid_vtableVt, soid_symbolVt, soid_primitiveMap, soid_symbolMapVt, soid_lobby, soid_arrayVt,

  soid__count
};

