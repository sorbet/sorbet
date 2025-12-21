# frozen_string_literal: true
# typed: true
# compiled: true

# Regression test for exception handling with captured variables.
# This tests that begin/rescue/ensure blocks can access captured variables
# from their enclosing scope.

def test_exception_captured_var
  result = 0
  previous = 10

  begin
    result = previous + 5
  rescue => e
    result = -1
  ensure
    result = result + previous
  end

  result
end

puts test_exception_captured_var
