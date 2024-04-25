# frozen_string_literal: true
# typed: true
# compiled: true

class A < T::Struct
  prop :simple, Integer
end

a = A.new(simple: 1129)
p a.simple
a.simple = 1099
p a.simple
