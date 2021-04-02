# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def initialize
    @a = 5
  end

  def foo
    @a
  end
end

obj = A.new
i = 0

while i < 1_000_000
  obj.foo
  i += 1
end

p i
