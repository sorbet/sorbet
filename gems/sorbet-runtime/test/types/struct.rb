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

  it "raises a TypeError setting nil on raise_on_nil_write prop" do
    klass = Class.new(T::Struct) do
      prop :foo, T.nilable(String), raise_on_nil_write: true
    end
    m = klass.new(foo: "baz")
    assert_raises(TypeError) do
      m.foo = nil
    end
  end

  it "allows omitting raise_on_nil_write prop on initialization" do
    klass = Class.new(T::Struct) do
      prop :foo, T.nilable(String), raise_on_nil_write: true
    end
    klass.new
  end

  it "allows explicitly setting nil on raise_on_nil_write prop on initialization" do
    klass = Class.new(T::Struct) do
      prop :foo, T.nilable(String), raise_on_nil_write: true
    end
    assert_raises(TypeError) do
      klass.new(foo: nil)
    end
  end

  it "allows explicit nil default for raise_on_nil_write prop" do
    klass = Class.new(T::Struct) do
      prop :foo, T.nilable(String), raise_on_nil_write: true, default: nil
    end
    m = klass.new
    assert(m.foo.nil?)
  end

  it "does not allow setting nil with T.any(NilClass, U) with raise_on_nil_write props" do
    klass = Class.new(T::Struct) do
      prop :foo, T.any(NilClass, String), raise_on_nil_write: true
    end
    m = klass.new
    assert_raises(TypeError) do
      m.foo = nil
    end
  end

  it "allows setting non-nil values with T.any(NilClass, U) with raise_on_nil_write props" do
    klass = Class.new(T::Struct) do
      prop :foo, T.any(NilClass, String), raise_on_nil_write: true
    end
    m = klass.new
    m.foo = 'foo'
  end

  it "allows setting nil values with T.nilable(NilClass) with raise_on_nil_write props, which is fine maybe?" do
    klass = Class.new(T::Struct) do
      prop :foo, T.nilable(NilClass), raise_on_nil_write: true
    end
    m = klass.new
    m.foo = nil
  end
end
