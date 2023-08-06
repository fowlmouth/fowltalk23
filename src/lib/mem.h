#pragma once

#include <cstdint>
#include <cstddef>

#include "object.h"

struct vtable_object;

class Memory
{

public:
  Memory();
  ~Memory();

  void* alloc(vtable_object* vtable, std::size_t size);
  void free(void* ptr);

  oop alloc_words(vtable_object* vt, std::size_t words);

};
