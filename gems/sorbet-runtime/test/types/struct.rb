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

  class SimpleStruct < T::Struct
    prop :value, Integer
  end

  class SimpleEnum < T::Enum
    enums do
      This = new
      That = new
    end
  end

  class Outer < T::Struct
    prop :struct, SimpleStruct
    prop :enum, SimpleEnum
  end

  describe "T::Struct#with" do
    it "works correctly when a value is a T::Struct" do
      c = Outer.new(struct: SimpleStruct.new(value: 5), enum: SimpleEnum::This)
      d = c.with(struct: SimpleStruct.new(value: 8))
      assert_equal(8, d.struct.value)
      # this should be unchanged by #with
      assert_equal(SimpleEnum::This, d.enum)
    end

    it "works correctly when a value is a T::Enum" do
      c = Outer.new(struct: SimpleStruct.new(value: 5), enum: SimpleEnum::This)
      d = c.with(enum: SimpleEnum::That)
      assert_equal(SimpleEnum::That, d.enum)
      # this should be unchanged by #with
      assert_equal(5, d.struct.value)
    end
  end
end
