#include "mem.h"

Memory::Memory()
{
}

Memory::~Memory()
{
}

void* Memory::alloc(vtable_object* vtable, std::size_t size)
{
  if(size < sizeof(void*))
  {
    size = sizeof(void*);
  }
  vtable_object** header = (vtable_object**)new char[sizeof(vtable_object*) + size];
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
