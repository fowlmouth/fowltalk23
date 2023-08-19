#include "mem.h"

struct MemorySpaceHeader
{
  void* begin;
  std::size_t size;
  void* next_alloc;
};

Memory::Memory(void* region_start, std::size_t region_size)
: region_start(region_start), region_size(region_size)
{
  MemorySpaceHeader* header = reinterpret_cast< MemorySpaceHeader* >(region_start);
  next_alloc = reinterpret_cast< void* >(header + 1);
  if(header->next_alloc)
  {
    next_alloc = header->next_alloc;
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
