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

  class UntypedField < T::Struct
    const :untyped_const, T.untyped, default: nil
    prop :untyped_prop, T.untyped, default: nil
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
    assert_raises(TypeError) do
      MyStruct.new(name: 100)
    end
  end

  it 'can pass objects to an array of subdocs' do
    o = Outer.new(objs: [Inner.new(i: 4), Inner.new(i: 5)])
    assert_equal([4, 5], o.objs.map(&:i))
  end

  it 'can default untyped fields' do
    UntypedField.new
  end

  class WeakConstructorCustomInitializeStruct
    include T::Props
    include T::Props::Serializable
    include T::Props::WeakConstructor

    prop :name, String
    prop :greeting, String, default: "Hi"
    prop :farewell, String, default: "Bye"
    prop :bool, T::Boolean
    prop :color, T.nilable(String)
    prop :type, T.nilable(String), raise_on_nil_write: true

    def initialize(hash={})
      @name = 'Doe'
      @greeting = nil
      @farewell = 'Ciao'
      @bool = false
      @color = 'red'
      @type = 'value'
      super
    end
  end

  it 'does not clobber custom initialize T::Props::WeakConstructor' do
    c = WeakConstructorCustomInitializeStruct.new
    assert_equal('Doe', c.name)
    assert_equal('Hi', c.greeting)
    assert_equal('Bye', c.farewell)
    assert_equal(false, c.bool)
    assert_equal('red', c.color)
    assert_equal('value', c.type)

    c = WeakConstructorCustomInitializeStruct.new(name: 'Alex', greeting: 'hello', farewell: 'goodbye', bool: true, color: 'blue', type: 'other')
    assert_equal('Alex', c.name)
    assert_equal('hello', c.greeting)
    assert_equal('goodbye', c.farewell)
    assert_equal(true, c.bool)
    assert_equal('blue', c.color)
    assert_equal('other', c.type)
  end

  class CustomInitializeStruct < T::Struct
    prop :name, String
    prop :greeting, String, default: "Hi"
    prop :farewell, String, default: "Bye"
    prop :bool, T::Boolean
    prop :color, T.nilable(String)
    prop :type, T.nilable(String), raise_on_nil_write: true

    def initialize(hash={})
      @name = 'Doe'
      @greeting = nil
      @farewell = 'Ciao'
      @bool = false
      @color = 'red'
      @type = 'value'
      super
    end
  end

  it 'does not clobber custom initialize for T::Struct' do
    c = CustomInitializeStruct.new(name: 'Alex', bool: true, type: 'other')
    assert_equal('Alex', c.name)
    assert_equal('Hi', c.greeting)
    assert_equal('Bye', c.farewell)
    assert_equal(true, c.bool)
    assert_nil(c.color)
    assert_equal('other', c.type)

    c = CustomInitializeStruct.new(name: 'Alex', greeting: 'hello', farewell: 'goodbye', bool: true, color: 'blue', type: 'other')
    assert_equal('Alex', c.name)
    assert_equal('hello', c.greeting)
    assert_equal('goodbye', c.farewell)
    assert_equal(true, c.bool)
    assert_equal('blue', c.color)
    assert_equal('other', c.type)

    err = assert_raises(ArgumentError) do
      CustomInitializeStruct.new(bool: true, type: 'other')
    end
    assert_equal("Missing required prop `name` for class `Opus::Types::Test::Props::ConstructorTest::CustomInitializeStruct`", err.message)
  end
end
