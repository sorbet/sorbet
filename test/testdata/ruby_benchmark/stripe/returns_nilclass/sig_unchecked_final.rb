# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig(:final) {returns(NilClass).checked(:never)}
def returns_nilclass
  nil
end

i = 0
while i < 10_000_000
  i += 1

  self.returns_nilclass
end

puts i
puts returns_nilclass
