#pragma once

#include "mem.h"

#include "object.h"

#include <cstdint>
#include <cstddef>

enum vtable_slot_flags
{
  vts_parent    = 0b001, // if this bit is 0 it is a normal data slot
  vts_static    = 0b010, // if this bit is 0 it lives in the instance
  vts_setter    = 0b100, // if this bit is 1 the slot is a setter

  vts_static_parent = vts_static | vts_parent,
  vts_data = 0,

  // these combinations are not supported
  vts_invalid1 = vts_static | vts_setter,              // static slot setters could be implemented sometime in the future
  vts_invalid2 = vts_parent | vts_static | vts_setter, // static parents should remain const, not settable IMO
  vts_invalid3 = vts_parent | vts_setter,              // pointless, would be no different than data slot setter

  vts__bit_count = 3,
  vts__mask = (1 << vts__bit_count) - 1
};

using string_ref = char*;

struct vtable_slot
{
protected:
  image_offset_t key_flags;
  oop value_;

public:
  vtable_slot(vtable_slot_flags, image_offset_t key, oop value);

  enum vtable_slot_flags flags() const;
  image_offset_t key() const;
  oop value() const;

  inline bool empty() const
  {
    return !key_flags;
  }
};

class Image;

struct vtable_object
{
  friend class Image;
private:
  // the capacity of slots, this is always a power of 2
  size_t slot_capacity, slot_count;

  // the number of static parents and instance parents in the vtable
  size_t static_parent_count, parent_count;

  // the size of the instance in words
  size_t instance_size_words;

  // activatable bytecode object, if this is a method frame
  oop bytecode_;

public:
  // next is array of slots
  vtable_slot* slots_begin();
  vtable_slot* slots_end();

  const vtable_slot* slots_begin() const;
  const vtable_slot* slots_end() const;

  // followed by the array of static parents
  oop* static_parents_begin();
  oop* static_parents_end();

  const oop* static_parents_begin() const;
  const oop* static_parents_end() const;

  static std::size_t calculate_vtable_size(std::size_t slot_capacity, std::size_t static_parent_count);
  static vtable_object* make(std::size_t slot_count, std::size_t static_parent_count, vtable_object* vtable_vt, Memory& mem);

  void* allocate_instance(Memory& mem);

  oop bytecode() const;
  void set_bytecode(oop new_bytecode);

  enum add_slot_result_t
  {
    asr_ok,
    asr_error_slot_exists,
    asr_error_invalid_type,
    asr_error_reorder_parents,
    asr_error_too_many_static_parents,
    asr_error_too_many_slots
  };
  enum
  {
    asr__count = asr_error_too_many_slots + 1
  };

  add_slot_result_t add_slot(Image&, const char* slot_name, vtable_slot_flags flags, oop value);

  bool lookup(Image& image, void* instance, string_ref symbol, vtable_slot_flags* slot_flags, oop& result) const;
  bool recursive_lookup(Image& image, void* instance, string_ref selector, oop& result, oop* owner, vtable_slot_flags* slot_flags) const;
};

oop object_vtable(void* object);
vtable_object* object_vtable(void* object, Memory& memory);
vtable_object* oop_vtable(oop object, Image& image);
