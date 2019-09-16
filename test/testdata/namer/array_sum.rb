# typed: strict

# We used to have code like this in pay-server, and it started failing after
# the change to introduce block arguments on all methods because of how we
# handled filling in arguments over top of intrinsic methods.

class Array
  extend T::Sig
  sig {returns(T.untyped)}
  def sum; end # error: Method `Array#sum` redefined without matching argument count. Expected: `1`, got: `0`
end
