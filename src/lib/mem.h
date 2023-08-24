#pragma once

#include <cstdint>
#include <cstddef>

#include "object.h"

struct vtable_object;

class Memory
{
protected:
  void* region_start, *next_alloc;
  std::size_t region_size;

public:
  Memory(void* region_start, std::size_t region_size, void* next_alloc);
  ~Memory();

  struct MemoryExhaustedError
  {
  };

  void* alloc(vtable_object* vtable, std::size_t size);
  void free(void* ptr);

  void* alloc_words(vtable_object* vt, std::size_t words);

  image_offset_t offset(void* ptr) const;
  void* ptr(image_offset_t offset) const;

};
