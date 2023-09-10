#pragma once

#include "vtable.h"
#include "mem.h"
#include "primitives.h"

#include <cstdlib>
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

  using add_slot_result_t = vtable_object::add_slot_result_t;
  add_slot_result_t add_slot(vtable_object* vtable, const char* slot_name, vtable_slot_flags flags, oop value);

  void replace_data(void*, std::size_t);
  void update_header();

  void set_vtable(void* object, vtable_object* new_vt);

  bool validate_header(void* region_ptr, std::size_t region_size) const;

public:
  Image(std::size_t image_size);
  Image(const char* path);
  ~Image();

  enum ImageLoadResult
  {
    ILR_OK,
    ILR_ErrorOpeningFile,
    ILR_ErrorMMapFailed,
    ILR_ErrorFileSizeMismatch
  };
  enum
  {
    ILR__count = ILR_ErrorFileSizeMismatch + 1
  };

  ImageLoadResult load(const char* filename);
  void save(const char* filename);

  unsigned int hash_symbol(string_ref symbol);

  void bootstrap();

  void add_entrypoint(oop method);

  using EntrypointCallback = bool(*)(oop, void*);
  // callback returns false to stop iteration
  void each_entrypoint(EntrypointCallback callback, void* arg) const;

  // TODO return oop
  string_ref intern(const char* symbol);

  oop special_object(std::size_t index) const;
};
