# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {returns(NilClass).checked(:never)}
def returns_nilclass
  nil
end

i = 0
while i < 10_000_000

  self.returns_nilclass

  i += 1
end

puts i
puts returns_nilclass
