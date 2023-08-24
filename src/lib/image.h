#pragma once

#include "vtable.h"
#include "mem.h"
#include "primitives.h"

#include <cstring>
#include <cstdint>
#include <iostream>
#include <iterator>

// OSX
#include <sys/mman.h>

struct TODO{};

class Image : public Memory
{
  object_array special_objects;

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

  add_slot_result_t add_slot(vtable_object* vtable, const char* slot_name, vtable_slot_flags flags, void* value);

  unsigned int hash_symbol(string_ref symbol);
  void replace_data(void*, std::size_t);
  void update_header();

  void set_vtable(void* object, vtable_object* new_vt);

public:
  Image(std::size_t image_size);
  ~Image();

  void load(const char* filename);
  void save(const char* filename);

  void bootstrap();

  string_ref intern(const char* symbol);

  oop special_object(std::size_t index) const;
};
