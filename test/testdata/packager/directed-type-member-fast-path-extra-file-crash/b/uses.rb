# typed: strict
# frozen_string_literal: true

# stratum: 1

class B::Uses
  extend T::Sig

  sig { returns(A::Gen[Integer, Integer, Integer, Integer]) }
  def self.gen = raise
end
