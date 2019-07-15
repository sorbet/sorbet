# typed: true

class A
  extend T::Generic

  S = type_member
end

module I
  extend T::Sig
  extend T::Helpers
  extend T::Generic

  interface!

  In = type_member(:in)
  Out = type_member(:out)

  # should pass: Out and In are used in invariant position
  sig {abstract.params(arg: A[Out]).returns(A[In])}
  def invariant_arg_and_return(arg); end

  # should pass: Out is used in an array used in invariant context
  sig {abstract.params(arg: A[T::Array[Out]]).void}
  def invariant_arg_array(arg); end

  # should pass: In is used in an array used in invariant context
  sig {abstract.returns(A[T::Array[In]])}
  def return_invariant_array; end
end
