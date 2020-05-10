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

  describe "#==" do
    class TestStruct < T::Struct
      prop :x, Integer
      prop :y, Float
    end

    class AnotherTestStruct < T::Struct
      prop :x, Integer
      prop :y, Float
    end

    it "is reflexively true" do
      a = TestStruct.new(x: 1, y: 0.2)

      assert_equal(a, a)
    end

    it "is true if all properties are equal" do
      a = TestStruct.new(x: 1, y: 4.2)
      b = TestStruct.new(x: 1, y: 4.2)

      assert_equal(a, b)
    end

    it "is false if objects are not the same class" do
      a = TestStruct.new(x: 1, y: 4.2)
      b = AnotherTestStruct.new(x: 1, y: 4.2)

      refute_equal(a, b)
    end

    it "is false if some properties are not equal" do
      a = TestStruct.new(x: 1, y: 4.2)
      b = TestStruct.new(x: 2, y: 1.1)

      refute_equal(a, b)
    end
  end
end
