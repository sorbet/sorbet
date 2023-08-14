# typed: true

ints = T::Array[Integer].new
strings = T::Array[String].new

x = ints + strings
T.reveal_type(x)

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
