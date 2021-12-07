# frozen_string_literal: true
# typed: true
# compiled: true

class Compiled
  extend T::Sig

  sig(:final) {returns(T::Array[Integer])}
  def test_instance_method
    [10,12]
  end

  sig(:final) {returns(T::Array[Integer])}
  def self.test_singleton_method
    [10,12]
  end
end
