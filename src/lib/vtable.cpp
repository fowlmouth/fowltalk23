#include "vtable.h"
#include "image.h"

vtable_slot::vtable_slot(vtable_slot_flags flags, image_offset_t key, oop value)
: key_flags((image_offset_t)((uintptr_t)key | ((uintptr_t)flags & vts__mask))), value_(value)
{
}

enum vtable_slot_flags vtable_slot::flags() const
{
  // the flags are stored in the lower 3 bits of the key_flags
  return (enum vtable_slot_flags)((uintptr_t)key_flags & vts__mask);
}

image_offset_t vtable_slot::key() const
{
  // the key is stored in the upper bits of the key_flags
  return (image_offset_t)((uintptr_t)key_flags & ~(uintptr_t)vts__mask);
}

oop vtable_slot::value() const
{
  return value_;
}



vtable_slot* vtable_object::slots_begin()
{
  return reinterpret_cast< vtable_slot* >(this + 1);
}
vtable_slot* vtable_object::slots_end()
{
  return slots_begin() + slot_capacity;
}

// followed by the array of static parents
oop* vtable_object::static_parents_begin()
{
  return (oop*)slots_end();
}
oop* vtable_object::static_parents_end()
{
  return static_parents_begin() + static_parent_count;
}




std::size_t vtable_object::calculate_vtable_size(std::size_t slot_capacity, std::size_t static_parent_count)
{
  return sizeof(vtable_object) + (slot_capacity * sizeof(vtable_slot)) + (static_parent_count * sizeof(oop));
}

uint32_t next_power_of_two(uint32_t v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

vtable_object* vtable_object::make(std::size_t slot_count, std::size_t static_parent_count, vtable_object* vtable_vt, Memory& mem)
{
  uint32_t capacity = next_power_of_two(slot_count);
  if(slot_count > capacity)
  {
    return nullptr;
  }
  std::size_t vtable_size = calculate_vtable_size(capacity, static_parent_count);
  vtable_object* vtable = (vtable_object*)mem.alloc(vtable_vt, vtable_size);
  vtable->slot_capacity = slot_count;
  vtable->slot_count = 0;
  vtable->static_parent_count = static_parent_count;
  vtable->parent_count = 0;
  vtable->instance_size_words = 0;

  return vtable;
}

vtable_object* oop_vtable(oop o)
{
  return (vtable_object*)*((void**)o - 1);
}

oop object_vtable(void* object)
{
  return ((oop*)object)[-1];
}

vtable_object* object_vtable(void* object, Memory& memory)
{
  return (vtable_object*)memory.ptr(object_vtable(object));
}

void* vtable_object::allocate_instance(Memory& mem)
{
  void* result = mem.alloc(this, instance_size_words * sizeof(oop));
  return result;
}

vtable_object::add_slot_result_t vtable_object::add_slot(Image& image, const char* slot_name, vtable_slot_flags flags, void* value_object)
{

  if(slot_count == slot_capacity)
  {
    return asr_error_too_many_slots;
  }

  flags = (vtable_slot_flags)(flags & vts__mask);

  string_ref symbol = image.intern(slot_name);
  oop symbol_offset = image.offset(symbol);
  const unsigned int slot_mask = slot_capacity - 1;
  auto hash = image.hash_symbol(symbol);
  unsigned int index = hash & slot_mask;
  vtable_slot* slot = slots_begin() + index;
  while(! slot->empty())
  {
    if(slot->key() == symbol_offset)
    {
      return asr_error_slot_exists;
    }
    index = (index + 1) & slot_mask;
    slot = slots_begin() + index;
  }

  oop value = image.offset(value_object);

  switch(flags)
  {
  case vts_static_parent:
  {
    // static parent, validate vtable can fit more static parents
    int static_parent_index = -1;
    for(std::size_t i = 0; i < static_parent_count; ++i)
    {
      if(! static_parents_begin()[i])
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
    static_parents_begin()[ static_parent_index ] = value;
    *slot = vtable_slot(flags, symbol_offset, int_to_oop(static_parent_index));

  } break;

  case vts_static:
    // static data slot
    *slot = vtable_slot(flags, symbol_offset, value);
    break;

  case vts_parent:

    if(instance_size_words > parent_count)
    {
      // non-parent slots already defined, cannot define parent slot
      return asr_error_reorder_parents;
    }
    ++parent_count;
    // fall-through: these are normal parent slots

  case vts_data:
  {
    // instance data slot
    auto instance_index = instance_size_words ++;
    *slot = vtable_slot(flags, symbol_offset, int_to_oop(instance_index));

  } break;


  default:
    return asr_error_invalid_type;

  }

  ++ slot_count;
  return asr_ok;
}
