# typed: true

# Sorbet does not take the fast path when a "large" number of files change (10 in the test suite).
# This test ensures that 1 fewer than this limit (9) still takes the fast path.

class MyClass
  def my_method
    a = T.let(10, String) # error: Argument does not have asserted type `String`
  end
end
