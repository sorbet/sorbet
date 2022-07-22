# typed: strict
extend T::Sig

class UpperBound; end

class Box < T::Struct
  extend T::Sig
  extend T::Generic

  Elem = type_member {{upper: UpperBound}}

  prop :val, Elem
end

sig do
  type_parameters(:U)
  .params(
    x: Box[T.type_parameter(:U)], # error: `T.type_parameter(:<todo typeargument>)` is not a subtype of upper bound of type member `::Box::Elem`
    y: T.type_parameter(:U),
  )
  .void
end
def example(x, y)
  T.let(y, UpperBound) # error: Argument does not have asserted type `UpperBound`

  x.val = y
end
