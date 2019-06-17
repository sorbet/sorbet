# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::StructValidationTest < Critic::Unit::UnitTest
  it "forbids subclassing a Struct" do
    c = Class.new(T::Struct)
    err = assert_raises(RuntimeError) do
      Class.new(c)
    end
    assert_includes(err.message, "is a subclass of T::Struct and cannot be subclassed")
  end

  it "forbids declaring a Struct as abstract" do
    err = assert_raises(RuntimeError) do
      Class.new(T::Struct) do
        extend T::Helpers
        abstract!
      end
    end
    assert_includes(err.message, "is a subclass of T::Struct and cannot be declared abstract")
  end
end
