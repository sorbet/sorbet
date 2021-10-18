# typed: true

ints = T::Array[Integer].new
strings = T::Array[String].new

ints + strings

ints + (strings)
ints + ((strings))
ints.+(strings)

ints.+(
  strings
)

ints +
  strings

# TODO(jez) Detect this, suggest `ints = ints.concat(strings)`
ints += strings
