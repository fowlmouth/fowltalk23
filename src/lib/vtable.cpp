#include "vtable.h"

vtable_slot::vtable_slot(vtable_slot_flags flags, string_ref key, oop value)
  : key_flags((string_ref)((uintptr_t)key | ((uintptr_t)flags & vts__mask))), value_(value)
{
}

enum vtable_slot_flags vtable_slot::flags() const
{
  // the flags are stored in the lower 3 bits of the key_flags
  return (enum vtable_slot_flags)((uintptr_t)key_flags & vts__mask);
}

string_ref vtable_slot::key() const
{
  // the key is stored in the upper bits of the key_flags
  return (string_ref)((uintptr_t)key_flags & ~(uintptr_t)vts__mask);
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

static void set_vtable(oop object, vtable_object* new_vt)
{
  // *(**void)object) - 1) = new_vt;
  ((object_array)object)[-1] = new_vt;
}

oop vtable_object::allocate_instance(Memory& mem)
{
  oop result = mem.alloc(this, instance_size_words * sizeof(oop));
  set_vtable(result, this);
  return result;
}


