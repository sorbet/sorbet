# typed: true

# Test precedence rules for 'it': local variable > block parameter > method
# Based on Ruby 3.4 spec:
#  - "If `it` is defined as a method, anonymous parameter takes precedence"
#  - "If `it` is available as a local variable, it takes precedence"
# Note: Method call disambiguation (like it(it)) is tested in it_param_method_vs_param.rb

class ItMethodTest
  extend T::Sig

  # Define 'it' as a method that returns String
  sig { returns(String) }
  def it
    "method_value"
  end

  sig { void }
  def test_method_precedence
    # Inside block, 'it' refers to block parameter, NOT the method
    # This works because 'it' is Integer from block
    result = [1, 2, 3].map { it * 2 }

    # Type error: 'it' is Integer (block param), not String (from method)
    [1, 2, 3].map { it.upcase } # error: Method `upcase` does not exist on `Integer`

    # To call the method, need explicit receiver
    [1, 2, 3].map { self.it.upcase }
  end

  sig { void }
  def test_local_variable_precedence
    # Local variable 'it' takes precedence over both method and block parameter
    it = "local_string"

    # 'it' refers to local variable (String), not block parameter (Integer)
    result = [1, 2, 3].map { it.upcase }

    # Type error: 'it' is String (local var), not Integer (block param)
    [1, 2, 3].map { it.times { } } # error: Method `times` does not exist on `String`

    # Can use String methods since 'it' is the local variable
    x = it + "test"
  end

  sig { void }
  def test_both_precedences
    # Local variable 'it' shadows both method and block parameter
    it = 100

    # 'it' is Integer(100), not Integer from block, not String from method
    ["a", "b", "c"].map { it + 50 } # No error, 'it' is 100

    # Type error: 'it' is Integer(100), can't call upcase
    ["a", "b", "c"].map { it.upcase } # error: Method `upcase` does not exist on `Integer`
  end
end

# Test at top level: method 'it' vs block parameter
def it
  "top_level_method"
end

# Block parameter takes precedence over method
[1, 2, 3].map { it * 2 } # No error, 'it' is Integer from block

# Type error if we expect method behavior
[1, 2, 3].map { it.upcase } # error: Method `upcase` does not exist on `Integer`

# Outside block, can call the method (no block parameter to shadow it)
x = it
y = it.length
