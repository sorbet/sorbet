# typed: strict

# We used to have code like this in pay-server, and it started failing after
# the change to introduce block arguments on all methods because of how we
# handled filling in arguments over top of intrinsic methods.
#
# A later improvement to namer made it so that this behavior is no longer an
# error. Rather than attempt to preserve the error, this example now no longer
# reports an error, and instead overwrites the type of one of the overloads of
# `Array#sum` (the overload that matches this arity).

class Array
  extend T::Sig
  sig {returns(T.untyped)}
  def sum; end
end

T.reveal_type(T::Array[Integer].new.sum) # error: `T.untyped`
