# frozen_string_literal: true
# typed: true
# compiled: true

class A
  extend T::Sig

  sig(:final) {void.checked(:never)}
  def self.final_method
  end
end

i = 0
while i < 10_000_000

  A.final_method

  i += 1
end

puts i
