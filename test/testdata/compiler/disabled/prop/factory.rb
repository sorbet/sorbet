# frozen_string_literal: true
# typed: true
# compiled: true

class A < T::Struct
  prop :foo, Integer, factory: -> {5}
end

a1 = A.new
p a1.foo
