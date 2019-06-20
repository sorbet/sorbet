# frozen_string_literal: true
require_relative '../../test_helper'

class Opus::Types::Test::Props::ConstructorTest < Critic::Unit::UnitTest
  class MyStruct < T::Struct
    prop :name, String
    prop :greeting, String, default: "Hi"
    prop :foo, T.nilable(T::Hash[T.any(String, Symbol), Object])
  end

  class Inner < T::Struct
    prop :i, Integer
  end

  class Outer < T::Struct
    prop :objs, T::Array[Inner]
  end

  it "raises when omitting a required prop" do
    err = assert_raises(ArgumentError) do
      MyStruct.new(foo: 'foo')
    end
    assert_equal("Missing required prop `name` for class `Opus::Types::Test::Props::ConstructorTest::MyStruct`", err.message)
  end

  it 'allows required props to be omitted if they have a default value' do
    m = MyStruct.new(name: "Alex",
                     foo: {color: :red})
    assert_equal("Hi", m.greeting)
  end

  it 'populates props' do
    m = MyStruct.new(name: "Alex",
                     foo: {color: :red})
    assert_equal("Alex", m.name)
    assert_equal("Hi", m.greeting)
    assert_equal({color: :red}, m.foo)
  end

  it 'raises on unknown props' do
    assert_raises(ArgumentError) do
      MyStruct.new(notathing: 4)
    end
  end

  it 'checks types' do
    assert_raises(T::Props::InvalidValueError) do
      MyStruct.new(name: 100)
    end
  end

  it 'can pass objects to an array of subdocs' do
    o = Outer.new(objs: [Inner.new(i: 4), Inner.new(i: 5)])
    assert_equal([4, 5], o.objs.map(&:i))
  end
end
