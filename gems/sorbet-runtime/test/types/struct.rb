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
    m = klass::new(foo: "baz")
    assert_raises(TypeError) do
      m.foo = nil
    end
  end

  # Failing - missing required prop
  it "raises a TypeError omitting raise_on_nil_write prop on initialization" do
    klass = Class.new(T::Struct) do
      prop :foo, T.nilable(String), raise_on_nil_write: true
    end
    klass::new
  end

  # Failing - cannot write nil, needs String
  it "raises a TypeError explicitly setting nil on raise_on_nil_write prop on initialization" do
    klass = Class.new(T::Struct) do
      prop :foo, T.nilable(String), raise_on_nil_write: true
    end
    klass::new(foo: nil)
  end

  it "allows explicit nil default for raise_on_nil_write prop" do
    klass = Class.new(T::Struct) do
      prop :foo, T.nilable(String), raise_on_nil_write: true, default: nil
    end
    m = klass.new
    assert(m.foo.nil?)
  end
end
