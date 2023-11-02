# typed: true

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

class Child < Parent # error: does not derive from `Class`
  def example; end
end
