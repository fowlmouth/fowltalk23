#include "mem.h"

Memory::Memory(void* region_start, std::size_t region_size, image_offset_t next_alloc)
: region_start(region_start), next_alloc(next_alloc), region_size(region_size)
{
}

Memory::~Memory()
{
}

#define ALIGN8(num) (((num) + 7) & ~7)

void* Memory::alloc(vtable_object* vtable, std::size_t size)
{
  size = ALIGN8(size+sizeof(oop));
  if(size < sizeof(oop)*2)
  {
    size = sizeof(oop)*2;
  }
  oop* header = (oop*)((char*)region_start + next_alloc);
  next_alloc += size;
  *header = offset(vtable);
  return (void*)(header+1);
}

void* Memory::alloc_words(vtable_object* vtable, std::size_t words)
{
  return alloc(vtable, words * sizeof(void*));
}

void Memory::free(void* ptr)
{
  (void)ptr;
}

image_offset_t Memory::offset(void* ptr) const
{
  return (image_offset_t)((char*)ptr - (char*)region_start);
}

void* Memory::ptr(image_offset_t offset) const
{
  return (char*)region_start + offset;
}
