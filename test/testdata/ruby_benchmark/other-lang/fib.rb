# frozen_string_literal: true
# typed: true
# compiled: true
def fib n
  if n < 3
    1
  else
    fib(n-1) + fib(n-2)
  end
end

fib(34)
