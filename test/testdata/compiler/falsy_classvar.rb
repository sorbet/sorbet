# frozen_string_literal: true
# compiled: true
# typed: true

# Ensure that @@var ||= ...'s expansion tests truthiness, not just defined-ness.

class A
  extend T::Sig

  @@falsy_var = T.let(false, T.any(FalseClass, T::Hash[T.untyped, T.untyped]))

  def self.init
    @@falsy_var ||= {}
  end
end

initial = A.init
other = A.init
p initial
p other
p initial.object_id == other.object_id
