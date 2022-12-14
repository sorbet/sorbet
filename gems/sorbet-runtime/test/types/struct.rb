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

  describe "immutable structs" do
    it "errors when using prop" do
      assert_raises(RuntimeError) do
        Class.new(T::ImmutableStruct) do
          prop :foo, Integer
        end
      end
    end

    it "errors when using with" do
      klass = Class.new(T::ImmutableStruct) do
        const :foo, Integer
      end

      assert_raises(RuntimeError) do
        klass.new(foo: 1).with({foo: 5})
      end
    end

    it "produces frozen objects" do
      klass = Class.new(T::ImmutableStruct) do
        const :foo, Integer
      end

      object = klass.new(foo: 5)

      assert_raises(FrozenError) do
        object.instance_variable_set(:@foo, 3)
      end
    end
  end

  class NestedStruct < T::Struct
    const :data, T::Hash[Symbol, String]
    const :sensitive, T.nilable(String), sensitivity: ['reason']
    const :custom, T.nilable(String), inspect: proc {|value, opts| "\"Inspected '#{value}' (opts: #{opts})\"" unless value.nil?}
    const :nested, T.nilable(NestedStruct)
  end

  module DecoratedClassName
    include T::Props::Plugin
    module DecoratorMethods
      extend T::Sig
      sig {params(instance: T::Props::PrettyPrintable).returns(String)}
      def inspect_class_with_decoration(instance)
        "#{instance.class}[decorated]"
      end
    end
  end

  class StructWithDecoratedName < T::Struct
    include DecoratedClassName
    const :data, String
  end

  describe "inspection" do
    def make_nested_struct
      NestedStruct.new(
        data: {
          one: "one",
          two: "two",
        },
        custom: "something",
        nested: NestedStruct.new(data: {three: "three"}, sensitive: "something sensitive")
      )
    end

    it "inspects in a single line" do
      struct = make_nested_struct
      expected_result = "<Opus::Types::Test::StructValidationTest::NestedStruct " \
      "custom=\"Inspected 'something' (opts: {:multiline=>false})\" data={:one=>\"one\", :two=>\"two\"} " \
      "nested=<Opus::Types::Test::StructValidationTest::NestedStruct custom=nil data={:three=>\"three\"} " \
      "nested=nil sensitive=<REDACTED reason>> sensitive=nil>"
      assert_equal(expected_result, struct.inspect)
    end

    it "pretty inspects" do
      struct = make_nested_struct
      expected_result = <<~INSPECT
        <Opus::Types::Test::StructValidationTest::NestedStruct
         custom="Inspected 'something' (opts: {:multiline=>true})"
         data={:one=>"one", :two=>"two"}
         nested=<Opus::Types::Test::StructValidationTest::NestedStruct
          custom=nil
          data={:three=>"three"}
          nested=nil
          sensitive=<REDACTED reason>>
         sensitive=nil>
      INSPECT
      assert_equal(expected_result, struct.pretty_inspect)
    end

    it "supports decorating the class name" do
      struct = StructWithDecoratedName.new(data: 'test')
      expected_result = "<Opus::Types::Test::StructValidationTest::StructWithDecoratedName[decorated] data=\"test\">"
      assert_equal(expected_result, struct.inspect)
    end
  end
end
