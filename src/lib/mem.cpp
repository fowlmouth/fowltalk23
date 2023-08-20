#include "mem.h"

Memory::Memory(void* region_start, std::size_t region_size, void* next_alloc)
: region_start(region_start), next_alloc(next_alloc), region_size(region_size)
{
  if(!next_alloc)
  {
    next_alloc = region_start;
  }
}

Memory::~Memory()
{
}

#define ALIGN8(num) (((num) + 7) & ~7)

void* Memory::alloc(vtable_object* vtable, std::size_t size)
{
  size = ALIGN8(size);
  if(size < sizeof(void*))
  {
    size = sizeof(void*);
  }
  vtable_object** header = (vtable_object**)next_alloc;
  next_alloc = (char*)next_alloc + sizeof(void*) + size;
  *header = vtable;
  return (oop)(header+1);
}

void* Memory::alloc_words(vtable_object* vtable, std::size_t words)
{
  return alloc(vtable, words * sizeof(void*));
}

void Memory::free(void* ptr)
{
  (void)ptr;
}
