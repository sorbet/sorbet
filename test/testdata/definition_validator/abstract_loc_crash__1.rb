# typed: strict

# This is a rather large comment at the start of the file which exposes a bug
# where we were accidentally turning a LocOffset from this file into a Loc for
# the other file. With a sufficiently long comment, that causes an array access
# out of bounds.
# This is a rather large comment at the start of the file which exposes a bug
# where we were accidentally turning a LocOffset from this file into a Loc for
# the other file. With a sufficiently long comment, that causes an array access
# out of bounds.
# This is a rather large comment at the start of the file which exposes a bug
# where we were accidentally turning a LocOffset from this file into a Loc for
# the other file. With a sufficiently long comment, that causes an array access
# out of bounds.

class Parent
  extend T::Sig
  extend T::Helpers
  abstract!

  sig { abstract.void }
  def example; end
end

# The canonical `loc()` of Child is here
class Child < Parent # error: Missing definition for abstract method
end
