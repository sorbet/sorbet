# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def foo
  end
end

a = A.new

i = 0
while i < 10_000_000

  a.foo

  i += 1
end

puts i
