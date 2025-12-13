# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def foo
    452
  end
end

nil_ = T.let(nil, T.nilable(A))
a = T.let(A.new, T.nilable(A))

i = 0
while i < 10_000_000

  nil_&.foo
  a&.foo

  i += 1
end

puts i
p nil_&.foo
p a&.foo
