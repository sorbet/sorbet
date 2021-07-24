# frozen_string_literal: true
# typed: true
# compiled: true

module B
  Y = 440
end

class A
  X = B::Y

  def self.foo
    X
  end
end

p A.foo
