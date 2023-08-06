#pragma once

using oop = void*;

using object_array = oop*;

// ptr array of ptr void

// void *(*foo[]);

#define oop_is_int(oop) ((intptr_t)oop & 1)

#define int_to_oop(i) ((oop)((((intptr_t)i) << 1) | 1))
#define oop_to_int(oop) ((intptr_t)oop >> 1)

