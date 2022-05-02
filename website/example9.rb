# typed: true

module IBox
  extend T::Sig
  extend T::Generic
  Elem = type_member(:out)

  sig {returns(Elem)}
  def val; @val; end
end

class Box
  extend T::Sig
  extend T::Generic

  include IBox
  Elem = type_member

  sig {params(val: Elem).void}
  def initialize(val)
    @val = T.let(val, Elem)
  end

  # Can't define this on IBox, because it uses `Elem` contravariantly
  sig {params(val: Elem).returns(Elem)}
  def val=(val); @val = val; end
end

# Create a box of strings
string_box = Box[String].new('')

# Can't use Box, because via Box the Elem is invariant
bad_object_box = T.let(string_box, Box[Object])

# If it's used as an IBox, this becomes a static error.
# (note: generics are unchecked at runtime; this won't raise)
object_box = T.let(string_box, IBox[Object])
object_box.val = 0
