# typed: false

-> { 123 }
-> (param) { param }
-> (optional_param = 123) { optional_param }

# lambdas with do/end
-> do 456 end
-> (param) do param end
-> (optional_param = 123) do optional_param end

# TODO: Delete when https://github.com/sorbet/sorbet/issues/9631 is fixed
-> { 890 }.chained_method_call()

# *Not* actually a lambda, but just regular calls to `Kernel#lambda` with a block argument.
lambda { 123 }

# Test that lambda arguments are translated correctly
method_with_lambda_arg :arg1, -> { 123 }

# Test lambda literals with numbered arguments
-> { _1 + _2 }

# Even with a class, the receiver is still Kernel
class C
  def method_returning_lambda
    -> { 123 }
  end
end

# Empty lambda parameters
->() { 1 + 2 }
