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

  it "does recursively type-check values in its constructor" do
    c = Class.new(T::Struct) do
      prop :arr, T::Array[String]
    end
    assert_raises(TypeError) do
      c.new(arr: [1, 2])
    end
  end

  it "does recursively type-check arguments to setters" do
    c = Class.new(T::Struct) do
      prop :arr, T::Array[String]
    end
    c = c.new(arr: ["foo, bar"])
    assert_raises(TypeError) do
      c.arr = [1, 2]
    end
  end

  it "allows disabling tests in tests" do
    c = Class.new(T::Struct) do
      checked(:tests)
      prop :arr, T::Array[String]
      const :other, T::Array[Integer]
    end
    c = c.new(arr: ["foo, bar"], other: ["foo", "bar"]) # doesn't throw
    c.arr = [1, 2] # doesn't throw
  end

  it "errors when using string typed props" do
    assert_raises(TypeError) do
      Class.new(T::Struct) do
        prop "foo", T::Array[String]
      end
    end
  end

  it "errors when using string typed const" do
    assert_raises(TypeError) do
      Class.new(T::Struct) do
        const "foo", T::Array[String]
      end
    end
  end
end
