# typed: strict
# enable-experimental-rbs-comments: true

#: [A, B, C] (A, B?, C) -> void
def type_params1(a, b, c)
  x = a #: A
  T.reveal_type(x) # error: Revealed type: `T.type_parameter(:A) (of Object#type_params1)`

  y = [] #: Array[B]
  T.reveal_type(y) # error: Revealed type: `T::Array[T.type_parameter(:B) (of Object#type_params1)]`

  z = nil #: C?
  T.reveal_type(z) # error: Revealed type: `T.nilable(T.type_parameter(:C) (of Object#type_params1))`
end

#: [A] (A?) -> A
def type_params2(a)
  a #: as A
end

#: [A] (A?) -> A
def type_params3(a)
  a #: as !nil
end
