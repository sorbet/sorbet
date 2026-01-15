# typed: true
class Array
  T::Sig::WithoutRuntime.sig {
    type_parameters(:U)
      .params(x: T.type_parameter(:U))
      .returns(T.type_parameter(:U))
    #  ^^^^^^^ error: The initialize method should always return void
  }
  def initialize(x) # error: redefined without matching
    x
  end
end

xs = T::Array[Integer].new("")
T.reveal_type(xs) # error: `T::Array[Integer]`

# Combination of "passed a block" and "initialize has type_parameters" means
# that the SolveConstraint will attempt to solve and instantiate the return
# type if we did not set returnTypeBeforeSolve directly to a type that does not
# have any T.type_parameter in it.
xs = T::Array[Integer].new("") {}
#                              ^^ error: does not take a block
T.reveal_type(xs) # error: `T::Array[Integer]`
