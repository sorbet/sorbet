# typed: true

module IBox
  extend T::Sig
  extend T::Generic
  Elem = type_member(:out)

  sig {params(val: Elem).void} # error: variance mismatch
  def initialize(val)
    @val = T.let(val, Elem)
  end

  sig {returns(Elem)}
  def val; @val; end

  sig {params(val: Elem).returns(Elem)} # error: variance mismatch
  def val=(val); @val = val; end
end

class Box
  extend T::Sig
  extend T::Generic

  include IBox
  Elem = type_member
end

# Create a box of strings
string_box = Box[String].new('')

# Can't use Box, because via Box the Elem is invariant
bad_object_box = T.let(string_box, Box[Object])

# Circumvent the type system by using IBox instead
object_box = T.let(string_box, IBox[Object])

# not a runtime error, because generics are unchecked!!
# this would runtime error in Java (for arrays not boxes, at least)
object_box.val = 0

# the user can read integers from a string box!
string_box.val # => 0!!
