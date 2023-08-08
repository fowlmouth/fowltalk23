#include "image.h"

Image::Image(Memory& mem)
: mem(mem)
{
}

Image Image::load(const char* filename, Memory& mem)
{
  (void)filename;
  
  return Image(mem);
}

void Image::save(const char* filename)
{
  (void)filename;
}

static inline void set_vtable(oop object, vtable_object* new_vt)
{
  // *((void**)object - 1) = new_vt;
  ((object_array)object)[-1] = new_vt;
}

void Image::bootstrap()
{
  // vtable vtable is used by 99% of vtables
  vtable_object* vtable_vt = vtable_object::make(32, 1, nullptr, mem);
  set_vtable(vtable_vt, vtable_vt);

  // "defaultBehavior" object
  vtable_object* root_vt = vtable_object::make(32, 0, vtable_vt, mem);
  oop root_object = mem.alloc(root_vt, 0);

  vtable_vt->static_parents_begin()[0] = root_object;

  // string primitive vt
  vtable_object* string_primitive_vt = vtable_object::make(32, 1, vtable_vt, mem);
  string_primitive_vt->static_parents_begin()[0] = root_object;

  // array primitive vt
  vtable_object* array_primitive_vt = vtable_object::make(32, 1, vtable_vt, mem);
  special_objects = (object_array)mem.alloc_words(array_primitive_vt, soid__count);
  special_objects[soid_symbolVt] = string_primitive_vt;
  // now strings can be interned for slots

  vtable_object* lobby_vt = vtable_object::make(128, 1, vtable_vt, mem);

  // holds a map with primitive info for the vm
  vtable_object* primitive_map_vt = vtable_object::make(pid__count, 0, vtable_vt, mem);

  oop lobby = mem.alloc(lobby_vt, 0);

  special_objects[soid_vtableVt] = vtable_vt;
  special_objects[soid_primitiveMap] = mem.alloc(primitive_map_vt, 0);
  special_objects[soid_lobby] = lobby;

  vtable_object* globals_vt = vtable_object::make(128, 0, vtable_vt, mem);
  oop globals = mem.alloc(globals_vt, 0);

  add_slot(lobby_vt, "Globals", vts_static_parent, globals);

  add_slot(globals_vt, "Lobby", vts_static, lobby);
  add_slot(globals_vt, "VTableVT", vts_static, vtable_vt);
  add_slot(globals_vt, "PrimitiveArrayVT", vts_static, array_primitive_vt);
  add_slot(globals_vt, "PrimitiveStringVT", vts_static, string_primitive_vt);
}

inline unsigned int djb2(const char* str)
{
  unsigned long hash = 5381;
  for(int c; (c = *str++); )
  {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  return hash;
}

unsigned int Image::hash_symbol(string_ref symbol)
{
  return djb2((const char*)symbol);
}

Image::add_slot_result_t Image::add_slot(vtable_object* vtable, const char* slot_name, vtable_slot_flags flags, oop value)
{
  if(vtable->slot_count == vtable->slot_capacity)
  {
    return asr_error_too_many_slots;
  }

  flags = (vtable_slot_flags)(flags & vts__mask);

  string_ref symbol = intern(slot_name);
  const unsigned int slot_mask = vtable->slot_capacity - 1;
  auto hash = hash_symbol(symbol);
  unsigned int index = hash & slot_mask;
  vtable_slot* slot = vtable->slots_begin() + index;
  while(! slot->empty())
  {
    if(slot->key() == symbol)
    {
      return asr_error_slot_exists;
    }
    index = (index + 1) & slot_mask;
    slot = vtable->slots_begin() + index;
  }

  switch(flags)
  {
  case vts_static_parent:
  {
    // static parent, validate vtable can fit more static parents
    int static_parent_index = -1;
    for(std::size_t i = 0; i < vtable->static_parent_count; ++i)
    {
      if(! vtable->static_parents_begin()[i])
      {
        static_parent_index = (int)i;
        break;
      }
    }
    if(static_parent_index == -1)
    {
      // no room for static parent
      return asr_error_too_many_static_parents;
    }
    vtable->static_parents_begin()[ static_parent_index ] = value;
    *slot = vtable_slot(flags, symbol, int_to_oop(static_parent_index));

  } break;

  case vts_static:
    // static data slot
    *slot = vtable_slot(flags, symbol, value);
    break;

  case vts_parent:

    if(vtable->instance_size_words > vtable->parent_count)
    {
      // non-parent slots already defined, cannot define parent slot
      return asr_error_reorder_parents;
    }
    ++vtable->parent_count;
    // fall-through: these are normal parent slots

  case vts_data:
  {
    // instance data slot
    auto instance_index = vtable->instance_size_words ++;
    *slot = vtable_slot(flags, symbol, int_to_oop(instance_index));

  } break;


  default:
    return asr_error_invalid_type;

  }

  ++ vtable->slot_count;
  return asr_ok;

}

string_ref Image::intern(const char* symbol)
{
  vtable_object* symbolVt = (vtable_object*)special_object(soid_symbolVt);
  auto len = strlen(symbol);
  char* new_symbol = (char*)mem.alloc(symbolVt, len + 1);
  strcpy(new_symbol, symbol);
  new_symbol[len] = 0;
  return new_symbol;
}

oop Image::special_object(std::size_t index) const
{
  return special_objects[index];
}

