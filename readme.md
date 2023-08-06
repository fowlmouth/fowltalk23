# FowlTalk

FowlTalk is a prototype-based programming language inspired by Self and Slate

```
Vector := {
  ## Parent slots
  parent* = Copyable.

  ## Data slots
  x := 0.
  y := 0.

  ## Methods return self by default
  x: x y: y [
    x = x.
    y = y.
  ].

  distanceSquared [
    ^ (x * x) + (y * y)
  ].

  distance [
    ^ distanceSquared sqrt
  ].
}

```

### Project Layout

This implementation is split into three parts, a Virtual Machine, a bootstrap compiler, and a library which supports both of them.


