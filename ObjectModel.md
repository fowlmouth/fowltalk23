# Fowltalk Object Model

FowlTalk's object model is heavily inspired by this paper https://courses.cs.washington.edu/courses/cse501/15sp/papers/chambers.pdf

### VTable

VTables are objects that contain the layout of an object. 

#### Layout

The VTable object has 3 sections. The bulk of the size is an open-addressed hashmap of slots.

* a header with info about the vtable itself and instances which use it
* a power-of-two sized array of slot objects
* an array of object handles which are "static parents"

```c++
struct vtable_header
{
  // the capacity of slots, this is always a power of 2
  size_t slot_capacity, slot_count;

  // the number of static parents and instance parents in the vtable
  size_t static_parent_count, parent_count;

  // the size of the instance in words
  size_t instance_size_words;

  // next is array of slots
  vtable_slot* slots_begin()
  {
    return (vtable_slot*)(this + 1);
  }
  vtable_slot* slots_end()
  {
    return slots_begin() + slot_capacity;
  }

  // followed by the array of static parents
  oop* static_parents()
  {
    return (oop*)slots_end();
  }
  oop* static_parents_end()
  {
    return static_parents_begin() + static_parent_count;
  }
};
```

#### Slots

```c++
enum vtable_slot_flags
{
  vts_parent    = 0b001, // if this bit is 0 it is a normal data slot
  vts_static    = 0b010, // if this bit is 0 it lives in the instance
  vts_setter    = 0b100, // if this bit is 1 the slot is a setter
  vts__bit_count = 3,
  vts__mask = (1 << vts__bit_count) - 1
};

struct vtable_slot
{
  string_object* key_flags;
  oop value;

  enum vtable_slot_flags flags() const
  {
    // the flags are stored in the lower 3 bits of the key_flags
    return (enum vtable_slot_flags)((uintptr_t)key_flags & vts__mask);
  }

  string_object* key() const
  {
    // the key is stored in the upper bits of the key_flags
    return (string_object*)((uintptr_t)key_flags & ~(uintptr_t)vts__mask);
  }
};
```

With this combination of bitflags the slots can live in either the instance or the parent

* parent slots can be either instance or static
* static parent setters are not valid, but instance parent setters are just fine
* for instance data slots "value" is an integer index into the object
* setter slots use the same index to identify their data slot
* for static parent slots "value" is an integer index into vtable "static parent" section

Parent slots are normal data slots. They do have to be ordered first.

## Code Example

```ft
Copyable := {
  copy [
    ^ _copy
  ]
}.

Point := {
  parent* = Copyable.

  x := 0.
  y := 0.

  squareDistance [
    ^ (x * x) + (y * y)
  ].

  distance [
    ^ squareDistance sqrt
  ]
}.
```

Copyable slots:
* copy - static data slot (an activatable method)

Point slots:
* parent - static parent slot
* x - instance data slot (0)
* x: - instance data setter (slot=0)
* y - instance data slot (1)
* y: - instance data setter (slot=1)
* squareDistance - static data slot (an activatable method)
* distance - static data slot (an activatable method)

