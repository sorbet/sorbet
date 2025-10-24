# typed: true

# Test method call disambiguation: calling methods named 'it' while using 'it' block parameter
# Note: Precedence rules are tested in it_param_method_vs_local.rb
# Note: RSpec/Minitest DSL integration is tested in it_param_rspec_method.rb and minitest_it_param.rb

class ItMethodCallTest
  extend T::Sig

  # Instance method named 'it' that takes a parameter
  sig {params(x: Integer).returns(Integer)}
  def it(x)
    x * 2
  end

  sig {void}
  def test_method_call_disambiguation
    # Can call method named 'it' with explicit argument
    result1 = it(5)
    T.reveal_type(result1) # error: Revealed type: `Integer`

    # Inside a block, 'it' without parens/args refers to block parameter
    result2 = [1, 2, 3].map { it * 2 }
    T.reveal_type(result2) # error: Revealed type: `T::Array[Integer]`

    # Key case: calling 'it' method with 'it' block parameter as argument
    result3 = [1, 2, 3].map { it(it) }
    # First 'it' (with parens) is method call, second 'it' is block parameter
    T.reveal_type(result3) # error: Revealed type: `T::Array[Integer]`

    # Can also use explicit self to call the method
    result4 = [1, 2, 3].map { self.it(it) }
    T.reveal_type(result4) # error: Revealed type: `T::Array[Integer]`
  end
end
