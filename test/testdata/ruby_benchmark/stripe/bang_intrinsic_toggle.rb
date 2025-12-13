# frozen_string_literal: true
# typed: true
# compiled: true

i = 0
x = T.let(true, T::Boolean)
while i < 10_000_000
  x = !x
  i += 1
end
puts i
puts x
