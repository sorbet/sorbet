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

ints + strings

ints += strings
ints += strings
ints += strings

[{"foo" => "bar"}] + [{}]

strings + ""
