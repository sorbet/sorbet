# frozen_string_literal: true

require_relative "../../test_helper"

class Opus::Types::Test::Props::ValueObjectTest < Critic::Unit::UnitTest
  class MyStruct < T::Struct
    include T::Props::ValueObject

    prop :a, String
    const :b, Integer
  end

  class Circular < T::Struct
    include T::Props::ValueObject

    prop :danger, T.nilable(Circular)
  end

  class IncludesProps
    include T::Props
    include T::Props::ValueObject

    prop :a, String
  end

  class Child < IncludesProps; end

  it "compares T::Structs based on their classes and props" do
    first = MyStruct.new(a: "hello", b: 5)
    second = MyStruct.new(a: "hello", b: 5)

    assert_equal(first, second)

    first.a = "bye"
    refute_equal(first, second)
  end

  it "takes class and props into account for calculating the object's hash" do
    first = MyStruct.new(a: "hello", b: 5)
    hash = [MyStruct, %i[a b], ["hello", 5]].hash

    assert_equal(hash, first.hash)
  end

  it "matches regular struct behavior for comparions" do
    first = MyStruct.new(a: "hello", b: 5)
    second = MyStruct.new(a: "hello", b: 5)

    assert_equal(0, first <=> second)

    first.a = "blop"
    assert_nil(first <=> second)
  end

  # If we have a circular dependency between two structs, we want to create an infinite loop as it happens in other
  # typed languages. The current implementation does result in an infinite loop, but Ruby eventually raises a
  # SystemStackError because the stack level is too deep.
  # See https://github.com/sorbet/sorbet/issues/1540#issuecomment-525978986
  it "creates an infinite loop if we have a circular reference" do
    first = Circular.new
    second = Circular.new(danger: first)
    first.danger = second

    assert_raises(SystemStackError, "stack level too deep") do
      first == second
    end
  end

  it "implements equality for objects that are not structs but include props" do
    first = IncludesProps.new
    second = IncludesProps.new
    first.a = "Parent"
    second.a = "Parent"

    assert_equal(first, second)
  end

  it "does not consider objects to be equal if their classes are different" do
    child = Child.new
    child.a = "secret"
    parent = IncludesProps.new
    parent.a = "secret"

    refute_equal(child, parent)
  end

  it "compares classes that inherit from value objects properly" do
    first = Child.new
    first.a = "secret"
    second = Child.new
    second.a = "secret"

    assert_equal(first, second)

    second.a = "different secret"
    refute_equal(first, second)
  end

  it "can be used as hash keys" do
    first = MyStruct.new(a: "hello", b: 5)
    second = MyStruct.new(a: "hello", b: 5)

    hash = {}
    hash[first] = "You found me!"

    assert_equal("You found me!", hash[second])
  end
end
