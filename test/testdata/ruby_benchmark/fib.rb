# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(n: Integer).returns(Integer).checked(:never)}
def fib(n)
  case n
  when 0 then 0
  when 1 then 1
  else fib(n - 1) + fib(n - 2)
  end
end

i = 0
while i < 1_000_000
  fib(5)
  i += 1
end

puts i
