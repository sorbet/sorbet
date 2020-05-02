# frozen_string_literal: true
require_relative '../../test_helper'

require 'parser/current'

class Opus::Types::Test::Props::SerializableTest < Critic::Unit::UnitTest
  def assert_prop_error(match, &blk)
    ex = assert_raises(ArgumentError) do
      Class.new do
        include T::Props::Serializable
        class_exec(&blk)
      end
    end
    assert_match(match, ex.message)
  end

  class MySerializable
    include T::Props::Serializable
    prop :name, T.nilable(String), raise_on_nil_write: true
    prop :foo, T.nilable(T::Hash[T.any(String, Symbol), Object])
  end

  def a_serializable
    m = MySerializable.new
    m.name = "Bob"
    m.foo  = {
      'age' => 7,
      'color' => 'red',
    }
    m
  end

  class DefaultsStruct
    include T::Props::Serializable
    include T::Props::WeakConstructor

    prop :prop1, T.nilable(String), default: "this is prop 1"
    prop :prop2, T.nilable(Integer), factory: -> {raise "don't call me"}
    prop :trueprop, T::Boolean, default: true
    prop :falseprop, T::Boolean, default: false
    prop :factoryprop, T::Boolean, factory: -> {true}
  end

  describe ':default and :factory' do
    it 'defaults do not override on from_hash' do
      m = DefaultsStruct.from_hash('prop1' => 'value',
                                   'prop2' => 17)
      assert_equal('value', m.prop1)
      assert_equal(17, m.prop2)
    end

    it 'defaults do not replace nil on from_hash' do
      m = DefaultsStruct.from_hash('prop1' => 'value')
      assert_nil(m.prop2)

      m = DefaultsStruct.from_hash('prop2' => 30)
      assert_nil(m.prop1)
    end

    it 'does not call factories on from_hash' do
      m = DefaultsStruct.from_hash({})
      assert_nil(m.prop2)
    end

    it 'does call default on required prop' do
      m = DefaultsStruct.from_hash({})
      assert(m.trueprop)
    end

    it 'does call factory on required prop' do
      m = DefaultsStruct.from_hash({})
      assert(m.factoryprop)
    end
  end

  it 'returns the right required props' do
    assert_equal(
      Set[:trueprop, :falseprop, :factoryprop],
      DefaultsStruct.decorator.required_props.to_set
    )
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

  it 'serializes' do
    m = a_serializable

    hash = m.serialize
    assert_kind_of(Hash, hash)
    assert_equal("Bob", hash['name'])
    assert_equal(7, hash['foo']['age'])
  end

  describe '.inspect' do
    it 'inspects' do
      obj = a_serializable
      str = obj.inspect
      assert_equal('<Opus::Types::Test::Props::SerializableTest::MySerializable foo={"age"=>7, "color"=>"red"}, name="Bob">', str)
    end

    it 'inspects with extra props' do
      obj = a_serializable
      obj = obj.class.from_hash(obj.serialize.merge('not_a_prop' => 'but_here_anyway'))
      str = obj.inspect
      assert_equal('<Opus::Types::Test::Props::SerializableTest::MySerializable foo={"age"=>7, "color"=>"red"}, name="Bob" @_extra_props=<not_a_prop="but_here_anyway">>', str)
    end
  end

  describe '.from_hash' do
    it 'round-trips' do
      m = a_serializable
      assert_equal(m.serialize, m.class.from_hash(m.serialize).serialize)
    end

    it 'round-trips extra props' do
      m = a_serializable
      input = m.serialize.merge('not_a_prop' => 'foo')
      assert_equal('foo', m.class.from_hash(input).serialize['not_a_prop'])
    end

    it 'does not call the constructor' do
      MySerializable.any_instance.expects(:new).never
      MySerializable.from_hash({})
    end
  end

  describe 'error message' do
    it 'is only soft-assert by default for prop deserialize error' do
      msg_string = nil
      extra_hash = nil
      T::Configuration.soft_assert_handler = proc do |msg, extra|
        msg_string = msg
        extra_hash = extra
      end

      result = MySerializable.from_hash({'foo' => "Won't respond like hash"})
      assert_equal("Won't respond like hash", result.foo)

      refute_nil(msg_string)
      refute_nil(extra_hash)
      storytime = extra_hash[:storytime]
      assert_equal(MySerializable, storytime[:klass])
      assert_equal(:foo, storytime[:prop])
      assert_equal("Won't respond like hash", storytime[:value])
    end

    it 'includes relevant generated code on deserialize when we raise' do
      T::Configuration.soft_assert_handler = proc do |msg, extra|
        raise "#{msg} #{extra.inspect}"
      end

      e = assert_raises(T::Props::InvalidValueError) do
        MySerializable.from_hash({'foo' => "Won't respond like hash"})
      end

      assert_includes(e.message, "undefined method `transform_values'")
      assert_includes(e.message, "foo")
      assert_includes(e.message, "val.transform_values {|v| T::Props::Utils.deep_clone_object(v)}")
    end

    it 'includes relevant generated code on serialize' do
      m = a_serializable
      m.instance_variable_set(:@foo, "Won't respond like hash")
      e = assert_raises(T::Props::InvalidValueError) do
        m.serialize
      end

      assert_includes(e.message, "undefined method `transform_values'")
      assert_includes(e.message, 'h["foo"] = @foo.transform_values {|v| T::Props::Utils.deep_clone_object(v)}')
    end
  end

  class HasUnstoredProp < T::Struct
    prop :stored, String
    prop :not_stored, String, dont_store: true
  end

  describe 'dont_store prop' do
    it 'is not stored' do
      m = HasUnstoredProp.new(stored: 'foo', not_stored: 'bar')
      assert_equal({'stored' => 'foo'}, m.serialize)
    end

    it 'does not prevent extra_props from round-tripping' do
      input = {'stored' => 'foo', 'not_a_prop' => 'bar'}
      result = HasUnstoredProp.from_hash(input).serialize
      assert_equal('bar', result['not_a_prop'])
    end

    it 'is allowed even on strict deserialize' do
      input = {'stored' => 'foo', 'not_stored' => 'bar'}
      refute_nil(HasUnstoredProp.from_hash!(input))
    end
  end

  describe 'hash props' do
    it 'does not share structure on serialize' do
      m = MySerializable.new
      m.name = 'hi'
      m.foo = {'hello' => {'world' => 1}}
      h = m.serialize
      refute_equal(m.foo.object_id, h['foo'].object_id, "`foo` is the same object")
      refute_equal(m.foo['hello'].object_id, h['foo']['hello'].object_id, "`foo.hello` is the same object")
    end

    it 'does not share structure on deserialize' do
      h = {
        'name' => 'hi',
        'foo' => {'hello' => {'world' => 1}},
      }
      m = MySerializable.from_hash(h)
      refute_equal(m.foo.object_id, h['foo'].object_id, "`foo` is the same object")
      refute_equal(m.foo['hello'].object_id, h['foo']['hello'].object_id, "`foo.hello` is the same object")
    end
  end

  describe "prop declaration" do
    it ':name must be a string' do
      assert_prop_error(/Invalid name in prop/) do
        prop :foo, String, name: :goats
      end
    end
  end

  class MyNilableSerializable
    include T::Props::Serializable
    prop :name, T.nilable(String)
    prop :address, T.nilable(String)
  end

  describe "nilable" do
    it 'can serialize nilables' do
      obj = MyNilableSerializable.new
      obj.name = 'Avi'
      assert_equal({'name' => 'Avi'}, obj.serialize)

      obj = MyNilableSerializable.new
      obj.name = nil
      assert_equal({}, obj.serialize)
    end

    it 'can serialize optionals' do
      obj = MyNilableSerializable.new
      obj.address = 'TEST'
      assert_equal({'address' => 'TEST'}, obj.serialize)

      obj = MyNilableSerializable.new
      obj.address = nil
      assert_equal({}, obj.serialize)
    end
  end

  class NilFieldStruct < T::Struct
    prop :foo, T.nilable(Integer), raise_on_nil_write: true
    prop :bar, T.nilable(String), raise_on_nil_write: true
  end

  class NilFieldWeakConstructor
    include T::Props::Serializable
    include T::Props::WeakConstructor
    prop :prop, T.nilable(String), raise_on_nil_write: true
  end

  class NilDefaultStruct < T::Struct
    prop :prop, T.nilable(String), raise_on_nil_write: true, default: nil
  end

  describe 'raise_on_nil_write' do
    it 'requires the value to be true' do
      assert_prop_error(/if specified must be `true`/) do
        prop :foo, T.nilable(String), raise_on_nil_write: false
      end

      assert_prop_error(/if specified must be `true`/) do
        prop :foo, T.nilable(String), raise_on_nil_write: 1
      end
    end

    it 'forbids nil in setter' do
      struct = NilFieldStruct.new(foo: 1, bar: 'something')
      ex = assert_raises(TypeError) do
        struct.foo = nil
      end
      assert_includes(ex.message, "Can't set Opus::Types::Test::Props::SerializableTest::NilFieldStruct.foo to nil (instance of NilClass) - need a Integer")
      ex = assert_raises(TypeError) do
        struct.bar = nil
      end
      assert_includes(ex.message, "Can't set Opus::Types::Test::Props::SerializableTest::NilFieldStruct.bar to nil (instance of NilClass) - need a String")
    end

    it 'forbids nil in constructor' do
      assert_raises(ArgumentError) do
        NilFieldStruct.new
      end
      assert_raises(TypeError) do
        NilFieldStruct.new(foo: nil, bar: nil)
      end
    end

    it 'throws exception on nil serialize' do
      foo = NilFieldStruct.allocate
      ex = assert_raises(T::Props::InvalidValueError) do
        foo.serialize
      end
      assert_includes(ex.message, 'Opus::Types::Test::Props::SerializableTest::NilFieldStruct.foo not set for non-optional prop')
    end

    it 'does not assert when strict=false' do
      foo = NilFieldStruct.allocate
      foo.serialize(false)
    end

    describe 'with weak constructor' do
      it 'forbids nil in setter' do
        struct = NilFieldWeakConstructor.new(prop: 'something')
        assert_raises(TypeError) do
          struct.prop = nil
        end
      end

      it 'allows implicit but forbids explicit nil in constructor' do
        struct = NilFieldWeakConstructor.new
        assert_nil(struct.prop)
        assert_raises(TypeError) do
          NilFieldWeakConstructor.new(prop: nil)
        end
      end

      it 'throws exception on nil serialize' do
        struct = NilFieldWeakConstructor.new
        assert_raises(T::Props::InvalidValueError) do
          struct.serialize
        end
      end
    end

    describe 'when default is nil' do
      # Is this intended? It seems like the behavior you'd want with T::Props::WeakConstructor,
      # not T::Props::Constructor / T::Struct
      it 'allows nil in constructor' do
        struct = NilDefaultStruct.new
        assert_nil(struct.prop)
        struct = NilDefaultStruct.new(prop: nil)
        assert_nil(struct.prop)
      end

      # Is this intended? It doesn't seem like the default should affect the behavior
      # of the setter
      it 'allows nil in setter' do
        struct = NilDefaultStruct.new(prop: 'something')
        struct.prop = nil
        assert_nil(struct.prop)
      end

      it 'throws exception on nil serialize' do
        struct = NilDefaultStruct.new
        assert_raises(T::Props::InvalidValueError) do
          struct.serialize
        end
      end
    end
  end

  class WithModel1 < T::Struct
    prop :foo, String
    prop :bar, T.nilable(Integer)
  end

  class WithModel2 < T::Struct
    prop :f1, String
    prop :f2, T.nilable(WithModel1)
  end

  describe 'with function' do
    it 'with simple fields' do
      a = WithModel1.new(foo: 'foo')
      b = a.with(bar: 10)

      assert_equal('foo', a.foo)
      assert_nil(a.bar)
      assert_equal('foo', b.foo)
      assert_equal(10, b.bar)
    end

    it 'with invalid fields' do
      a = WithModel1.new(foo: 'foo')
      e = assert_raises(ArgumentError) do
        a.with(non_bar: 10)
      end
      assert_equal('Unexpected arguments: input({:non_bar=>10}), unexpected({"non_bar"=>10})', e.to_s)

      a = WithModel1.from_hash({'foo' => 'foo', 'foo1' => 'foo1'})
      e = assert_raises(ArgumentError) do
        a.with(non_bar: 10)
      end
      assert_equal('Unexpected arguments: input({:non_bar=>10}), unexpected({"non_bar"=>10})', e.to_s)
    end

    it 'with overwrite fields' do
      a = WithModel1.new(foo: 'foo', bar: 10)
      b = a.with(bar: 20)

      assert_equal('foo', a.foo)
      assert_equal(10, a.bar)
      assert_equal('foo', b.foo)
      assert_equal(20, b.bar)
    end

    it 'with nested fields' do
      a = WithModel2.new(f1: 'foo')
      b = a.with(f2: {foo: 'foo', bar: 10})

      assert_equal('foo', a.f1)
      assert_nil(a.f2)
      assert_equal('foo', b.f1)
      refute_nil(b.f2)
      assert_equal('foo', b.f2.foo)
      assert_equal(10, b.f2.bar)
    end
  end

  class BooleanStruct < T::Struct
    prop :prop, T::Boolean
    prop :nilable_prop, T.nilable(T::Boolean)
  end

  describe 'boolean props' do
    it 'are not cloned on serde' do
      T::Props::Utils.expects(:deep_clone_object).never

      s = BooleanStruct.new(prop: true)
      assert_equal(true, BooleanStruct.from_hash(s.serialize).prop)
    end
  end

  class HeterogenousUnionStruct < T::Struct
    prop :prop, T.any(String, MySerializable)
  end

  describe 'heterogenous union props' do
    it 'are cloned on serde' do
      T::Props::Utils.expects(:deep_clone_object).with('foo').at_least_once.returns('foo')

      s = HeterogenousUnionStruct.new(prop: 'foo')
      assert_equal('foo', HeterogenousUnionStruct.from_hash(s.serialize).prop)
    end
  end

  class MultipleStructUnionStruct < T::Struct
    prop :prop, T.any(MySerializable, MyNilableSerializable)
  end

  describe 'unions of two different serializables' do
    it 'are just cloned on serde' do
      obj = MyNilableSerializable.new
      T::Props::Utils.expects(:deep_clone_object).with(obj).at_least_once.returns(obj)

      s = MultipleStructUnionStruct.new(prop: obj)
      assert_equal(obj, MultipleStructUnionStruct.from_hash(s.serialize).prop)
    end
  end

  class CustomType
    extend T::Props::CustomType

    attr_accessor :value

    def self.deserialize(value)
      result = new
      result.value = value.clone.freeze
      result
    end

    def self.serialize(instance)
      instance.value
    end
  end

  class CustomTypeStruct < T::Struct
    prop :single, T.nilable(CustomType)
    prop :array, T::Array[CustomType], default: []
    prop :hash_key, T::Hash[CustomType, String], default: {}
    prop :hash_value, T::Hash[String, CustomType], default: {}
    prop :hash_both, T::Hash[CustomType, CustomType], default: {}
  end

  describe 'custom type' do
    it 'round trips as plain value' do
      assert_equal('foo', CustomTypeStruct.from_hash({'single' => 'foo'}).serialize['single'])
    end

    it 'round trips as array value' do
      assert_equal(['foo'], CustomTypeStruct.from_hash({'array' => ['foo']}).serialize['array'])
    end

    it 'round trips as hash key' do
      assert_equal({'foo' => 'bar'}, CustomTypeStruct.from_hash({'hash_key' => {'foo' => 'bar'}}).serialize['hash_key'])
    end

    it 'round trips as hash value' do
      assert_equal({'foo' => 'bar'}, CustomTypeStruct.from_hash({'hash_value' => {'foo' => 'bar'}}).serialize['hash_value'])
    end

    it 'round trips as hash key and value' do
      assert_equal({'foo' => 'bar'}, CustomTypeStruct.from_hash({'hash_both' => {'foo' => 'bar'}}).serialize['hash_both'])
    end
  end

  class MyEnum < T::Enum
    enums do
      FOO = new
      BAR = new
    end
  end

  class EnumStruct < T::Struct
    prop :enum, MyEnum
    prop :enum_of_enums, T.nilable(T.enum([MyEnum::BAR]))
  end

  describe 'enum' do
    it 'round trips' do
      s = EnumStruct.new(enum: MyEnum::FOO, enum_of_enums: MyEnum::BAR)

      serialized = s.serialize
      assert_equal('foo', serialized['enum'])
      assert_equal('bar', serialized['enum_of_enums'])

      roundtripped = EnumStruct.from_hash(serialized)
      assert_equal(MyEnum::FOO, roundtripped.enum)
      assert_equal(MyEnum::BAR, roundtripped.enum_of_enums)
    end
  end

  class SuperStruct
    include T::Props::Serializable

    prop :superprop, T.nilable(String)
  end

  module Mixin
    include T::Props::Serializable
    prop :mixinprop, T.nilable(String)
  end

  # In the style of ActiveSupport::Concern
  module Concern
    def self.included(other)
      other.instance_exec do
        prop :concernprop, T.nilable(String)
      end
    end
  end

  class SubStruct < SuperStruct
    include Mixin
    include Concern
    prop :subprop, T.nilable(String)
  end

  describe 'with inheritance' do
    it 'round trips parent prop' do
      assert_equal('foo', SubStruct.from_hash('superprop' => 'foo').serialize['superprop'])
    end

    it 'round trips child prop' do
      assert_equal('foo', SubStruct.from_hash('subprop' => 'foo').serialize['subprop'])
    end

    it 'round trips prop from T::Props::Serializable mixin' do
      assert_equal('foo', SubStruct.from_hash('mixinprop' => 'foo').serialize['mixinprop'])
    end

    it 'round trips prop from mixin which uses def self.included' do
      assert_equal('foo', SubStruct.from_hash('concernprop' => 'foo').serialize['concernprop'])
    end
  end

  class ReopenedSuperStruct; end

  class ReopenedSuperStruct
    include T::Props::Serializable

    prop :superprop, T.nilable(String)
  end

  class ReopenedSubStruct < ReopenedSuperStruct
    include Mixin
    include Concern
  end

  class ReopenedSubStruct < ReopenedSuperStruct
    prop :subprop, T.nilable(String)
  end

  describe 'with reopening' do
    it 'round trips parent prop' do
      assert_equal('foo', ReopenedSubStruct.from_hash('superprop' => 'foo').serialize['superprop'])
    end

    it 'round trips child prop' do
      assert_equal('foo', ReopenedSubStruct.from_hash('subprop' => 'foo').serialize['subprop'])
    end

    it 'round trips prop from T::Props::Serializable mixin' do
      assert_equal('foo', ReopenedSubStruct.from_hash('mixinprop' => 'foo').serialize['mixinprop'])
    end

    it 'round trips prop from mixin which uses def self.included' do
      assert_equal('foo', ReopenedSubStruct.from_hash('concernprop' => 'foo').serialize['concernprop'])
    end
  end

  class NoPropsStruct < T::Struct
  end

  describe 'struct without props' do
    it 'round trips' do
      assert_equal({}, NoPropsStruct.from_hash({}).serialize)
    end
  end

  class CustomSerializedForm < T::Struct
    prop :prop, String, name: 'something_custom'
  end

  describe 'custom serialized form' do
    it 'round trips' do
      hash = CustomSerializedForm.new(prop: 'foo').serialize
      assert_equal({'something_custom' => 'foo'}, hash)
      assert_equal('foo', CustomSerializedForm.from_hash(hash).prop)
    end
  end

  class ArrayOfNilableStruct < T::Struct
    prop :prop, T::Array[T.nilable(MyEnum)]
  end

  describe 'with array of nilable enums' do
    it 'can deserialize non-nil' do
      assert_equal([MyEnum::FOO], ArrayOfNilableStruct.from_hash('prop' => ['foo']).prop)
    end

    it 'can deserialize nil' do
      assert_equal([nil], ArrayOfNilableStruct.from_hash('prop' => [nil]).prop)
    end
  end

  class ModulePropStruct < T::Struct
    module NonScalar
    end

    class ConcreteNonScalar
      include NonScalar
    end

    module Scalar
    end

    class ConcreteScalar
      include Scalar
    end

    prop :nonscalar, T.nilable(NonScalar)
    prop :scalar, T.nilable(Scalar)
  end

  describe 'with a module prop type' do
    before do
      T::Configuration.scalar_types += [ModulePropStruct::Scalar.name]
    end

    after do
      T::Configuration.scalar_types = nil
    end

    it 'is cloned on serde by default' do
      val = ModulePropStruct::ConcreteNonScalar.new
      T::Props::Utils.expects(:deep_clone_object).with(val).returns(val).at_least_once

      s = ModulePropStruct.new(nonscalar: val)
      assert_equal(val, ModulePropStruct.from_hash(s.serialize).nonscalar)
    end

    it 'is not cloned on serde if set as scalar' do
      val = ModulePropStruct::ConcreteScalar.new
      T::Props::Utils.expects(:deep_clone_object).never

      s = ModulePropStruct.new(scalar: val)
      assert_equal(val, ModulePropStruct.from_hash(s.serialize).scalar)
    end
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
    prop :custom_type, CustomType
    prop :nilable_custom_type, T.nilable(CustomType)
    prop :default_custom_type, CustomType, default: CustomType.new
    prop :array_of_custom_type, T::Array[CustomType]
    prop :hash_of_custom_type_to_substruct, T::Hash[CustomType, MySerializable]
    prop :unidentified_type, Object
    prop :nilable_unidentified_type, T.nilable(Object)
    prop :array_of_unidentified_type, T::Array[Object]
    prop :defaulted_unidentified_type, Object, default: Object.new
    prop :hash_with_unidentified_types, T::Hash[Object, Object]
    prop :infinity_float, Float, default: Float::INFINITY
  end

  describe 'generated code' do
    describe 'serialize' do
      it 'validates' do
        src = ComplexStruct.decorator.send(:generate_serialize_source).to_s
        T::Props::GeneratedCodeValidation.validate_serialize(src)
      end

      it 'has meaningful validation which complains at lurky method invocation' do
        src = ComplexStruct.decorator.send(:generate_serialize_source).to_s
        src = src.sub(/\.transform_values\b/, '.something_suspicious')
        assert_raises(T::Props::GeneratedCodeValidation::ValidationError) do
          T::Props::GeneratedCodeValidation.validate_serialize(src)
        end
      end
    end

    describe 'deserialize' do
      it 'validates' do
        src = ComplexStruct.decorator.send(:generate_deserialize_source).to_s
        T::Props::GeneratedCodeValidation.validate_deserialize(src)
      end

      it 'has meaningful validation which complains at lurky method invocation' do
        src = ComplexStruct.decorator.send(:generate_deserialize_source).to_s
        src = src.sub(/\.default\b/, '.something_suspicious')
        assert_raises(T::Props::GeneratedCodeValidation::ValidationError) do
          T::Props::GeneratedCodeValidation.validate_deserialize(src)
        end
      end
    end

    describe 'disabling evaluation' do
      it 'works' do
        T::Props::HasLazilySpecializedMethods.disable_lazy_evaluation!

        m = Class.new do
          include T::Props::Serializable

          prop :foo, T.nilable(String)
        end

        assert_raises(T::Props::HasLazilySpecializedMethods::SourceEvaluationDisabled) do
          m.new.serialize
        end

        # Explicit call is still ok
        m.decorator.eagerly_define_lazy_methods!
      end

      after do
        T::Props::HasLazilySpecializedMethods.remove_instance_variable(:@lazy_evaluation_disabled)
      end
    end
  end
end
