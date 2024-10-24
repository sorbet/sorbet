# typed: false

-> { 123 }
-> (param) { param }
-> (optional_param = 123) { optional_param }
-> { 456 }.call
lambda { 789 }
lambda { |param| param }

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
