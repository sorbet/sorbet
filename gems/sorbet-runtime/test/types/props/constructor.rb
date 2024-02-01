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

  class NoProps < T::Struct
  end

  it "raises when omitting a required prop" do
    err = assert_raises(ArgumentError) do
      MyStruct.new(foo: 'foo')
    end
    assert_equal("Missing required prop `name` for class `Opus::Types::Test::Props::ConstructorTest::MyStruct`", err.message)
  end

  it "raises when giving unknown props" do
    err = assert_raises(ArgumentError) do
      MyStruct.new(name: 'Alex', foo: {}, bar1: 'bar1', bar2: 'bar2')
    end
    assert_equal("Opus::Types::Test::Props::ConstructorTest::MyStruct: Unrecognized properties: bar1, bar2", err.message)
  end

  it "raises when giving unknown props to a no-prop class" do
    err = assert_raises(ArgumentError) do
      NoProps.new(bar1: 'bar1', bar2: 'bar2')
    end
    assert_equal("Opus::Types::Test::Props::ConstructorTest::NoProps: Unrecognized properties: bar1, bar2", err.message)
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

  class WeakConstructorCustomInitializeStructInitializeBeforeProps
    include T::Props
    include T::Props::Serializable
    include T::Props::WeakConstructor

    def initialize(hash={})
      @name = 'Doe'
      @greeting = nil
      @farewell = 'Ciao'
      @bool = false
      @color = 'red'
      @type = 'value'
      super
    end

    prop :name, String
    prop :greeting, String, default: "Hi"
    prop :farewell, String, default: "Bye"
    prop :bool, T::Boolean
    prop :color, T.nilable(String)
    prop :type, T.nilable(String), raise_on_nil_write: true
  end

  class WeakConstructorCustomInitializeStructInitializeBetweenProps
    include T::Props
    include T::Props::Serializable
    include T::Props::WeakConstructor

    prop :name, String
    prop :greeting, String, default: "Hi"
    prop :farewell, String, default: "Bye"

    def initialize(hash={})
      @name = 'Doe'
      @greeting = nil
      @farewell = 'Ciao'
      @bool = false
      @color = 'red'
      @type = 'value'
      super
    end

    prop :bool, T::Boolean
    prop :color, T.nilable(String)
    prop :type, T.nilable(String), raise_on_nil_write: true
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

    c = WeakConstructorCustomInitializeStructInitializeBeforeProps.new
    assert_equal('Doe', c.name)
    assert_equal('Hi', c.greeting)
    assert_equal('Bye', c.farewell)
    assert_equal(false, c.bool)
    assert_equal('red', c.color)
    assert_equal('value', c.type)

    c = WeakConstructorCustomInitializeStructInitializeBeforeProps.new(name: 'Alex', greeting: 'hello', farewell: 'goodbye', bool: true, color: 'blue', type: 'other')
    assert_equal('Alex', c.name)
    assert_equal('hello', c.greeting)
    assert_equal('goodbye', c.farewell)
    assert_equal(true, c.bool)
    assert_equal('blue', c.color)
    assert_equal('other', c.type)

    c = WeakConstructorCustomInitializeStructInitializeBetweenProps.new
    assert_equal('Doe', c.name)
    assert_equal('Hi', c.greeting)
    assert_equal('Bye', c.farewell)
    assert_equal(false, c.bool)
    assert_equal('red', c.color)
    assert_equal('value', c.type)

    c = WeakConstructorCustomInitializeStructInitializeBetweenProps.new(name: 'Alex', greeting: 'hello', farewell: 'goodbye', bool: true, color: 'blue', type: 'other')
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

  class CustomInitializeStructInitializeBeforeProps < T::Struct
    def initialize(hash={})
      @name = 'Doe'
      @greeting = nil
      @farewell = 'Ciao'
      @bool = false
      @color = 'red'
      @type = 'value'
      super
    end

    prop :name, String
    prop :greeting, String, default: "Hi"
    prop :farewell, String, default: "Bye"
    prop :bool, T::Boolean
    prop :color, T.nilable(String)
    prop :type, T.nilable(String), raise_on_nil_write: true
  end

  class CustomInitializeStructInitializeBetweenProps < T::Struct
    prop :name, String
    prop :greeting, String, default: "Hi"
    prop :farewell, String, default: "Bye"

    def initialize(hash={})
      @name = 'Doe'
      @greeting = nil
      @farewell = 'Ciao'
      @bool = false
      @color = 'red'
      @type = 'value'
      super
    end

    prop :bool, T::Boolean
    prop :color, T.nilable(String)
    prop :type, T.nilable(String), raise_on_nil_write: true
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

    c = CustomInitializeStructInitializeBeforeProps.new(name: 'Alex', bool: true, type: 'other')
    assert_equal('Alex', c.name)
    assert_equal('Hi', c.greeting)
    assert_equal('Bye', c.farewell)
    assert_equal(true, c.bool)
    assert_nil(c.color)
    assert_equal('other', c.type)

    c = CustomInitializeStructInitializeBeforeProps.new(name: 'Alex', greeting: 'hello', farewell: 'goodbye', bool: true, color: 'blue', type: 'other')
    assert_equal('Alex', c.name)
    assert_equal('hello', c.greeting)
    assert_equal('goodbye', c.farewell)
    assert_equal(true, c.bool)
    assert_equal('blue', c.color)
    assert_equal('other', c.type)

    err = assert_raises(ArgumentError) do
      CustomInitializeStructInitializeBeforeProps.new(bool: true, type: 'other')
    end
    assert_equal("Missing required prop `name` for class `Opus::Types::Test::Props::ConstructorTest::CustomInitializeStructInitializeBeforeProps`", err.message)

    c = CustomInitializeStructInitializeBetweenProps.new(name: 'Alex', bool: true, type: 'other')
    assert_equal('Alex', c.name)
    assert_equal('Hi', c.greeting)
    assert_equal('Bye', c.farewell)
    assert_equal(true, c.bool)
    assert_nil(c.color)
    assert_equal('other', c.type)

    c = CustomInitializeStructInitializeBetweenProps.new(name: 'Alex', greeting: 'hello', farewell: 'goodbye', bool: true, color: 'blue', type: 'other')
    assert_equal('Alex', c.name)
    assert_equal('hello', c.greeting)
    assert_equal('goodbye', c.farewell)
    assert_equal(true, c.bool)
    assert_equal('blue', c.color)
    assert_equal('other', c.type)

    err = assert_raises(ArgumentError) do
      CustomInitializeStructInitializeBetweenProps.new(bool: true, type: 'other')
    end
    assert_equal("Missing required prop `name` for class `Opus::Types::Test::Props::ConstructorTest::CustomInitializeStructInitializeBetweenProps`", err.message)
  end

  class DefaultsStruct < T::Struct
    prop :prop1, T.nilable(String), default: "this is prop 1"
    prop :prop2, T.nilable(Integer), factory: -> {123}
    prop :trueprop, T::Boolean, default: true
    prop :falseprop, T::Boolean, default: false
    prop :factoryprop, T::Boolean, factory: -> {true}

    prop :untyped_prop1, T.untyped, default: nil
    const :untyped_const1, T.untyped, default: nil
  end

  class InvalidDefaultsStruct < T::Struct
    prop :default_nilable, T.nilable(T::Boolean), default: 1
    prop :default_non_nilable, T::Boolean, default: 1
    prop :factory_nilable, T.nilable(T::Boolean), factory: -> {1}
    prop :factory_non_nilable, T::Boolean, factory: -> {1}
  end

  class NilDefaultRequired < T::Struct
    const :integer_const_nil_default, Integer, default: nil
    prop :integer_prop_nil_default, Integer, default: nil
  end

  describe ':default and :factory' do
    it 'defaults do not override on given values' do
      m = DefaultsStruct.new(prop1: 'value', prop2: 17)
      assert_equal('value', m.prop1)
      assert_equal(17, m.prop2)
    end

    it 'defaults and factories replace missing keys' do
      m = DefaultsStruct.new(prop1: nil, prop2: nil)
      assert_nil(m.prop1)
      assert_nil(m.prop2)
      assert_equal(true, m.trueprop)
      assert_equal(true, m.factoryprop)

      m = DefaultsStruct.new
      assert_equal('this is prop 1', m.prop1)
      assert_equal(123, m.prop2)
      assert_equal(true, m.trueprop)
      assert_equal(true, m.factoryprop)
    end

    it 'fixed defaults are not type-checked, factory defaults are type-checked' do
      m = InvalidDefaultsStruct.new(factory_nilable: false, factory_non_nilable: false)
      assert_equal(1, m.default_nilable)
      assert_equal(1, m.default_non_nilable)
      assert_equal(false, m.factory_nilable)
      assert_equal(false, m.factory_non_nilable)

      err = assert_raises(TypeError) do
        InvalidDefaultsStruct.new
      end
      assert_includes(err.message, "Can't set Opus::Types::Test::Props::ConstructorTest::InvalidDefaultsStruct.factory_nilable to 1 (instance of Integer) - need a T::Boolean")

      err = assert_raises(TypeError) do
        InvalidDefaultsStruct.new(default_nilable: 1)
      end
      assert_includes(err.message, "Can't set Opus::Types::Test::Props::ConstructorTest::InvalidDefaultsStruct.default_nilable to 1 (instance of Integer) - need a T::Boolean")

      err = assert_raises(TypeError) do
        InvalidDefaultsStruct.new(factory_nilable: 2)
      end
      assert_includes(err.message, "Can't set Opus::Types::Test::Props::ConstructorTest::InvalidDefaultsStruct.factory_nilable to 2 (instance of Integer) - need a T::Boolean")
    end
  end

  it 'returns the right required props' do
    assert_equal(
      Set[:trueprop, :falseprop, :factoryprop],
      DefaultsStruct.decorator.required_props.to_set
    )

    assert_equal(
      Set[:integer_const_nil_default, :integer_prop_nil_default],
      NilDefaultRequired.decorator.required_props.to_set
    )
  end

  class DefaultStringStruct < T::Struct
    prop :stringprop_mutable, String, default: String.new('value') # rubocop:disable Performance/UnfreezeString
    prop :stringprop_frozen, String, default: 'value'
  end

  class DefaultArrayStruct < T::Struct
    prop :arrayprop, T::Array[String], default: []
    prop :arrayprop_with_values, T::Array[String], default: [1]
  end

  class DefaultHashStruct < T::Struct
    prop :hashprop, T::Hash[String, String], default: {}
    prop :hashprop_with_values, T::Hash[String, String], default: {1 => 1}
  end

  describe 'default: with literals' do
    it 'does not share structure for mutable strings' do
      a = DefaultStringStruct.new
      b = DefaultStringStruct.new

      refute_equal(a.stringprop_mutable.object_id, b.stringprop_mutable.object_id)
    end

    it 'shares structure for frozen strings' do
      a = DefaultStringStruct.new
      b = DefaultStringStruct.new

      assert_equal(a.stringprop_frozen.object_id, b.stringprop_frozen.object_id)
    end

    it 'does not share structure for empty arrays' do
      a = DefaultArrayStruct.new
      b = DefaultArrayStruct.new

      refute_equal(a.arrayprop.object_id, b.arrayprop.object_id)
    end

    it 'does not share for populated arrays' do
      a = DefaultArrayStruct.new
      b = DefaultArrayStruct.new

      refute_equal(a.arrayprop_with_values.object_id, b.arrayprop_with_values.object_id)
    end

    it 'does not share structure for empty hashes' do
      a = DefaultHashStruct.new
      b = DefaultHashStruct.new

      refute_equal(a.hashprop.object_id, b.hashprop.object_id)
    end

    it 'does not share for populated hashes' do
      a = DefaultHashStruct.new
      b = DefaultHashStruct.new

      refute_equal(a.hashprop_with_values.object_id, b.hashprop_with_values.object_id)
    end
  end

  class ParentWithNoDefault < T::InexactStruct
    prop :prop, String
  end

  class ChildWithDefault < ParentWithNoDefault
    prop :prop, String, default: '', override: true
  end

  it 'uses default set on child in constructor' do
    assert_equal('', ChildWithDefault.new.prop)
  end

  class NilFieldStruct < T::Struct
    prop :foo, T.nilable(Integer), raise_on_nil_write: true
    prop :bar, T.nilable(String), raise_on_nil_write: true
    prop :required, String, raise_on_nil_write: true
  end

  class UntypedStruct < T::Struct
    prop :untyped, T.untyped
    prop :untyped_with_default, T.untyped, default: 123
    prop :untyped_with_raise_on_nil_write, T.untyped, raise_on_nil_write: true
  end

  it 'forbids nil in constructor if raise_on_nil_write=true' do
    err = assert_raises(ArgumentError) do
      NilFieldStruct.new
    end
    assert_equal("Missing required prop `foo` for class `Opus::Types::Test::Props::ConstructorTest::NilFieldStruct`", err.message)

    err = assert_raises(TypeError) do
      NilFieldStruct.new(foo: nil, bar: nil, required: nil)
    end
    assert_includes(err.message, "Can't set Opus::Types::Test::Props::ConstructorTest::NilFieldStruct.foo to nil (instance of NilClass) - need a Integer")

    err = assert_raises(TypeError) do
      NilFieldStruct.new(foo: 1, bar: 'hey', required: nil)
    end
    assert_includes(err.message, "Can't set Opus::Types::Test::Props::ConstructorTest::NilFieldStruct.required to nil (instance of NilClass) - need a String")
  end

  it 'does not forbid nil in constructor for T.untyped' do
    UntypedStruct.new
    UntypedStruct.new(untyped: nil)
    UntypedStruct.new(untyped_with_default: true)
    UntypedStruct.new(untyped_with_raise_on_nil_write: nil)
    UntypedStruct.new(untyped: nil, untyped_with_default: nil, untyped_with_raise_on_nil_write: nil)
  end

  it 'returns the right required props for T.untyped' do
    assert_equal(
      Set[:untyped, :untyped_with_default, :untyped_with_raise_on_nil_write],
      UntypedStruct.decorator.required_props.to_set
    )
  end

  class SetterValidate < T::Struct
    prop :nilable_validated, T.nilable(Integer), setter_validate: ->(_prop, _value) {raise Error.new 'nilable_validated invalid'}
    prop :default_validated, Integer, default: 1, setter_validate: ->(_prop, _value) {raise Error.new 'default_validated invalid'}
  end

  class SetterValidateUntyped < T::Struct
    prop :untyped, T.untyped, setter_validate: ->(_prop, _value) {raise Error.new 'untyped invalid'}
  end

  class SetterValidateRaiseOnNilWrite < T::Struct
    prop :raise_on_nil_write, T.nilable(Integer), setter_validate: ->(_prop, _value) {raise Error.new 'raise_on_nil_write invalid'}, raise_on_nil_write: true
  end

  describe 'setter_validate' do
    it 'does not run when a nilable is nil' do
      SetterValidate.new
      SetterValidate.new(nilable_validated: nil)
    end

    it 'runs when a nilable is non-nil' do
      err = assert_raises {SetterValidate.new(nilable_validated: 5)}
      assert_equal('nilable_validated invalid', err.message)
    end

    it 'runs on T.untyped' do
      err = assert_raises {SetterValidateUntyped.new}
      assert_equal('untyped invalid', err.message)

      err = assert_raises {SetterValidateUntyped.new(untyped: nil)}
      assert_equal('untyped invalid', err.message)

      err = assert_raises {SetterValidateUntyped.new(untyped: 123)}
      assert_equal('untyped invalid', err.message)
    end

    it 'runs when raise_on_nil_write=true' do
      err = assert_raises(ArgumentError) do
        SetterValidateRaiseOnNilWrite.new
      end
      assert_equal("Missing required prop `raise_on_nil_write` for class `Opus::Types::Test::Props::ConstructorTest::SetterValidateRaiseOnNilWrite`", err.message)

      err = assert_raises {SetterValidateRaiseOnNilWrite.new(raise_on_nil_write: 5)}
      assert_equal('raise_on_nil_write invalid', err.message)

      err = assert_raises {SetterValidateRaiseOnNilWrite.new(raise_on_nil_write: nil)}
      assert_includes(err.message, "Can't set Opus::Types::Test::Props::ConstructorTest::SetterValidateRaiseOnNilWrite.raise_on_nil_write to nil (instance of NilClass) - need a Integer")
    end
  end

  class MyEnum < T::Enum
    enums do
      FooOne = new
      FooTwo = new
    end
  end

  class MySerializable < T::Struct
    prop :name, T.nilable(String)
    prop :foo, T.nilable(T::Hash[T.any(String, Symbol), Object])
  end

  class ComplexStruct < T::Struct
    prop :primitive, Integer
    prop :nilable, T.nilable(Integer)
    prop :nilable_on_read, T.nilable(Integer), raise_on_nil_write: true
    prop :primitive_default, Integer, default: 0
    prop :primitive_nilable_default, T.nilable(Integer), default: 0
    prop :factory, Integer, factory: -> {0}
    prop :primitive_array, T::Array[Integer]
    prop :array_default, T::Array[Integer], default: []
    prop :primitive_hash, T::Hash[String, Integer]
    prop :array_of_nilable, T::Array[T.nilable(Integer)]
    prop :nilable_array, T.nilable(T::Array[Integer])
    prop :substruct, MySerializable
    prop :nilable_substract, T.nilable(MySerializable)
    prop :default_substruct, MySerializable, default: MySerializable.new
    prop :array_of_substruct, T::Array[MySerializable]
    prop :hash_of_substruct, T::Hash[String, MySerializable]
    prop :infinity_float, Float, default: Float::INFINITY
    prop :negative_infinity_float, Float, default: -Float::INFINITY
    prop :nan_float, Float, default: Float::NAN
    prop :enum, MyEnum
    prop :nilable_enum, T.nilable(MyEnum)
    prop :default_enum, MyEnum, default: MyEnum::FooOne
    prop :deprecated_enum, Symbol, enum: %i[foo_one foo_two]
    prop :nilable_deprecated_enum, T.nilable(Symbol), enum: %i[foo_one foo_two]
    prop :default_deprecated_enum, Symbol, enum: %i[foo_one foo_two], default: :foo_one
  end

  it 'can construct complex object' do
    attributes = {
      primitive: 1,
      nilable_on_read: 1,
      primitive_array: [1],
      primitive_hash: {'1' => 1},
      array_of_nilable: [1, nil],
      substruct: MySerializable.new(name: 'foo1'),
      array_of_substruct: [MySerializable.new(name: 'foo2')],
      hash_of_substruct: {'3' => MySerializable.new(name: 'foo3')},
      enum: MyEnum::FooOne,
      deprecated_enum: :foo_one,
    }

    assert_equal(
      {
        "primitive" => 1,
        "nilable_on_read" => 1,
        "primitive_default" => 0,
        "primitive_nilable_default" => 0,
        "factory" => 0,
        "primitive_array" => [1],
        "array_default" => [],
        "primitive_hash" => {"1" => 1},
        "array_of_nilable" => [1, nil],
        "substruct" => {"name" => "foo1"},
        "default_substruct" => {},
        "array_of_substruct" => [{"name" => "foo2"}],
        "hash_of_substruct" => {"3" => {"name" => "foo3"}},
        "infinity_float" => Float::INFINITY,
        "negative_infinity_float" => -Float::INFINITY,
        "nan_float" => Float::NAN,
        "enum" => "fooone",
        "default_enum" => "fooone",
        "deprecated_enum" => :foo_one,
        "default_deprecated_enum" => :foo_one,
      },
      ComplexStruct.new(attributes).serialize
    )
  end
end
