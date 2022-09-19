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

    prop :untyped_prop1, T.untyped, default: nil
    const :untyped_const1, T.untyped, default: nil
  end

  class NilDefaultRequired < T::Struct
    const :integer_const_nil_default, Integer, default: nil
    prop :integer_prop_nil_default, Integer, default: nil
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

    assert_equal(
      Set[:integer_const_nil_default, :integer_prop_nil_default],
      NilDefaultRequired.decorator.required_props.to_set
    )
  end

  class DefaultArrayStruct
    include T::Props::Serializable
    include T::Props::WeakConstructor

    prop :arrayprop, T::Array[String], default: []
  end

  class DefaultHashStruct
    include T::Props::Serializable
    include T::Props::WeakConstructor

    prop :hashprop, T::Hash[String, String], default: {}
  end

  describe 'default: with literals' do
    it 'does not share structure for arrays' do
      a = DefaultArrayStruct.new
      b = DefaultArrayStruct.new

      refute_equal(a.arrayprop.object_id, b.arrayprop.object_id)
    end

    it 'does not share structure for hashes' do
      a = DefaultHashStruct.new
      b = DefaultHashStruct.new

      refute_equal(a.hashprop.object_id, b.hashprop.object_id)
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

    it 'inspects frozen structs' do
      obj = a_serializable.freeze
      str = obj.inspect
      assert_equal('<Opus::Types::Test::Props::SerializableTest::MySerializable foo={"age"=>7, "color"=>"red"}, name="Bob">', str)
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

      e = assert_raises(RuntimeError) do
        MySerializable.from_hash({'foo' => "Won't respond like hash"})
      end

      assert_includes(e.message, "undefined method `transform_values'")
      assert_includes(e.message, "foo")
      assert_includes(e.message, "val.transform_values {|v| T::Props::Utils.deep_clone_object(v)}")
    end

    it 'includes relevant generated code on serialize' do
      m = a_serializable
      m.instance_variable_set(:@foo, "Won't respond like hash")
      e = assert_raises(NoMethodError) do
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

  class NilSingleFieldStruct < T::Struct
    prop :foo, T.nilable(Integer), raise_on_nil_write: true
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
      ex = assert_raises(TypeError) do
        foo.serialize
      end
      assert_includes(ex.message, 'Opus::Types::Test::Props::SerializableTest::NilFieldStruct.foo not set for non-optional prop')
    end

    it 'does not assert when strict=false' do
      foo = NilFieldStruct.allocate
      foo.serialize(false)
    end

    it 'records the non-presence of required properties on deserialize' do
      struct = NilSingleFieldStruct.from_hash({})
      assert_nil(struct.foo)
      missing_props = struct.instance_variable_get(:@_required_props_missing_from_deserialize)
      refute_nil(missing_props)
      refute(missing_props.empty?)
      assert_equal(1, missing_props.length)
      assert(missing_props.include?(:foo))
    end

    it 'round-trips when required properties are not provided to deserialize' do
      struct = NilSingleFieldStruct.from_hash({})
      assert_nil(struct.foo)

      msg_string = nil
      extra_hash = nil

      T::Configuration.log_info_handler = proc do |msg, extra|
        msg_string = msg
        extra_hash = extra
      end

      h = struct.serialize

      assert_instance_of(Hash, h)
      assert(h.empty?)
      refute_nil(msg_string)
      refute_nil(extra_hash)
      assert_includes(msg_string, "missing required property in serialize")
      assert_equal(:foo, extra_hash[:prop])
      assert_includes(extra_hash[:class], "NilSingleFieldStruct")
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
        assert_raises(TypeError) do
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
        assert_raises(TypeError) do
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

  class StructWithTuples < T::Struct
    prop :unary_tuple, [Symbol]
    prop :tuple, [Integer, String]
    prop :triple, [Float, TrueClass, FalseClass]
    prop :array_of_tuple, T::Array[[Integer, String]]
    prop :tuple_with_combinator, [T.nilable(String)]
    prop :combinator_with_tuple, T.nilable([Symbol, Float])
  end

  describe 'tuples' do
    it 'typechecks' do
      exn = assert_raises(TypeError) do
        StructWithTuples.new(
          unary_tuple: ['not a symbol'],
          tuple: [0, ''],
          triple: [0.0, true, false],
          array_of_tuple: [[0, '']],
          tuple_with_combinator: [nil]
        )
      end
      assert_includes(exn.message, '.unary_tuple to ["not a symbol"] (instance of Array) - need a [Symbol]')
    end

    it 'roundtrips' do
      expected = StructWithTuples.new(
        unary_tuple: [:hello],
        tuple: [0, ''],
        triple: [0.0, true, false],
        array_of_tuple: [[0, '']],
        tuple_with_combinator: [nil]
      )

      actual = StructWithTuples.from_hash(expected.serialize)

      assert_equal(expected.unary_tuple, actual.unary_tuple)
      assert_equal(expected.tuple, actual.tuple)
      assert_equal(expected.triple, actual.triple)
      assert_equal(expected.array_of_tuple, actual.array_of_tuple)
      assert_equal(expected.tuple_with_combinator, actual.tuple_with_combinator)
      assert_nil(actual.combinator_with_tuple)
    end
  end

  class StructWithShapes < T::Struct
    prop :empty_shape, {}
    prop :symbol_key_shape, {foo: Integer}
    prop :string_key_shape, {'foo' => Integer}
  end

  describe 'shapes' do
    it 'typechecks' do
      exn = assert_raises(TypeError) do
        StructWithShapes.new(
          empty_shape: {},
          symbol_key_shape: {foo: 0},
          string_key_shape: {not_a_string: 0}
        )
      end
      assert_includes(exn.message, '.string_key_shape to {:not_a_string=>0} (instance of Hash) - need a {"foo" => Integer}')
    end

    it 'roundtrips' do
      expected = StructWithShapes.new(
        empty_shape: {},
        symbol_key_shape: {foo: 0},
        string_key_shape: {'foo' => 0}
      )

      actual = StructWithShapes.from_hash(expected.serialize)

      assert_equal(expected.empty_shape, actual.empty_shape)
      assert_equal(expected.symbol_key_shape, actual.symbol_key_shape)
      assert_equal(expected.string_key_shape, actual.string_key_shape)
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

  class CustomTypeWrapper
    include T::Props::Serializable
    prop :struct, CustomTypeStruct
  end

  describe 'custom type' do
    it 'round trips as plain value' do
      assert_equal('foo', CustomTypeStruct.from_hash({'single' => 'foo'}).serialize['single'])
    end

    it 'round trips with arrays' do
      a = %w[bar baz]
      assert_equal(a, CustomTypeStruct.from_hash({'array' => a}).serialize['array'])
    end

    it 'raises serialize errors when props with a custom subtype store the wrong datatype' do
      struct = CustomTypeStruct.new
      struct.instance_variable_set(:@single, 'not a serializable thing')
      e = assert_raises(TypeError) do
        struct.serialize
      end

      assert_includes(e.message, "Expected type T::Props::CustomType")
    end

    # It's hard to write tests for a CustomType with a custom deserialize method, so skip that.

    it 'raises serialize errors when props with a serializable subtype store the wrong datatype' do
      struct = CustomTypeWrapper.new
      struct.instance_variable_set(:@struct, 'not a serializable thing')
      e = assert_raises(NoMethodError) do
        struct.serialize
      end

      assert_includes(e.message, "undefined method `serialize'")
    end

    it 'raises deserialize errors when props with a serializable subtype store the wrong datatype' do
      obj = 'not a serializable thing'
      e = assert_raises(TypeError) do
        CustomTypeWrapper.from_hash({'struct' => obj})
      end

      assert_includes(e.message, 'provided to from_hash')
    end

    it 'round trips as array value' do
      assert_equal(['foo'], CustomTypeStruct.from_hash({'array' => ['foo']}).serialize['array'])
    end

    it 'raises serialize errors when props with an array of a custom subtype store the wrong datatype' do
      obj = CustomType.new
      struct = CustomTypeStruct.new
      struct.instance_variable_set(:@array, obj)
      e = assert_raises(NoMethodError) do
        struct.serialize
      end

      assert_includes(e.message, "undefined method `map'")
    end

    it 'raises deserialize errors when props with an array of a custom subtype store the wrong datatype' do
      msg_string = nil
      extra_hash = nil
      T::Configuration.soft_assert_handler = proc do |msg, extra|
        msg_string = msg
        extra_hash = extra
      end

      obj = CustomType.new
      result = CustomTypeStruct.from_hash({'array' => obj})
      assert_equal(obj, result.array)

      refute_nil(msg_string)
      refute_nil(extra_hash)
      storytime = extra_hash[:storytime]
      assert_equal(CustomTypeStruct, storytime[:klass])
      assert_equal(:array, storytime[:prop])
      assert_equal(obj, storytime[:value])
      assert_includes(storytime[:error], "undefined method `map'")
    end

    it 'round trips as hash key' do
      assert_equal({'foo' => 'bar'}, CustomTypeStruct.from_hash({'hash_key' => {'foo' => 'bar'}}).serialize['hash_key'])
    end

    it 'raises serialize errors when props with a hash with keys of a custom subtype store the wrong datatype' do
      obj = CustomType.new
      struct = CustomTypeStruct.new
      struct.instance_variable_set(:@hash_key, obj)
      e = assert_raises(NoMethodError) do
        struct.serialize
      end

      assert_includes(e.message, "undefined method `transform_keys'")
    end

    it 'raises deserialize errors when props with a hash with keys of a custom subtype store the wrong datatype' do
      msg_string = nil
      extra_hash = nil
      T::Configuration.soft_assert_handler = proc do |msg, extra|
        msg_string = msg
        extra_hash = extra
      end

      obj = 'not a hash'
      result = CustomTypeStruct.from_hash({'hash_key' => obj})
      assert_equal('not a hash', result.hash_key)

      refute_nil(msg_string)
      refute_nil(extra_hash)
      storytime = extra_hash[:storytime]
      assert_equal(CustomTypeStruct, storytime[:klass])
      assert_equal(:hash_key, storytime[:prop])
      assert_equal(obj, storytime[:value])
      assert_includes(storytime[:error], "undefined method `transform_keys'")
    end

    it 'round trips as hash value' do
      assert_equal({'foo' => 'bar'}, CustomTypeStruct.from_hash({'hash_value' => {'foo' => 'bar'}}).serialize['hash_value'])
    end

    it 'raises serialize errors when props with a hash with values of a custom subtype store the wrong datatype' do
      obj = CustomType.new
      struct = CustomTypeStruct.new
      struct.instance_variable_set(:@hash_value, obj)
      e = assert_raises(NoMethodError) do
        struct.serialize
      end

      assert_includes(e.message, "undefined method `transform_values'")
    end

    it 'raises deserialize errors when props with a hash with values of a custom subtype store the wrong datatype' do
      msg_string = nil
      extra_hash = nil
      T::Configuration.soft_assert_handler = proc do |msg, extra|
        msg_string = msg
        extra_hash = extra
      end

      obj = 'not a hash'
      result = CustomTypeStruct.from_hash({'hash_value' => obj})
      assert_equal('not a hash', result.hash_value)

      refute_nil(msg_string)
      refute_nil(extra_hash)
      storytime = extra_hash[:storytime]
      assert_equal(CustomTypeStruct, storytime[:klass])
      assert_equal(:hash_value, storytime[:prop])
      assert_equal(obj, storytime[:value])
      assert_includes(storytime[:error], "undefined method `transform_values'")
    end

    it 'round trips as hash key and value' do
      assert_equal({'foo' => 'bar'}, CustomTypeStruct.from_hash({'hash_both' => {'foo' => 'bar'}}).serialize['hash_both'])
    end

    it 'raises serialize errors when props with a hash with keys/values of a custom subtype store the wrong datatype' do
      obj = CustomType.new
      struct = CustomTypeStruct.new
      struct.instance_variable_set(:@hash_both, obj)
      e = assert_raises(NoMethodError) do
        struct.serialize
      end

      assert_includes(e.message, "undefined method `each_with_object'")
    end

    it 'raises deserialize errors when props with a hash with keys/values of a custom subtype store the wrong datatype' do
      msg_string = nil
      extra_hash = nil
      T::Configuration.soft_assert_handler = proc do |msg, extra|
        msg_string = msg
        extra_hash = extra
      end

      obj = 'not a hash'
      result = CustomTypeStruct.from_hash({'hash_both' => obj})
      assert_equal('not a hash', result.hash_both)

      refute_nil(msg_string)
      refute_nil(extra_hash)
      storytime = extra_hash[:storytime]
      assert_equal(CustomTypeStruct, storytime[:klass])
      assert_equal(:hash_both, storytime[:prop])
      assert_equal(obj, storytime[:value])
      assert_includes(storytime[:error], "undefined method `each_with_object'")
    end
  end

  class SetPropStruct
    include T::Props::Serializable
    prop :set, T::Set[String]
  end

  describe 'set props' do
    it 'round trips' do
      set = Set['foo']
      struct = SetPropStruct.new
      struct.set = set

      h = struct.serialize
      assert_equal(set, h['set'])

      roundtripped = SetPropStruct.from_hash(h)
      assert_equal(roundtripped.set, set)
    end

    it 'does not share structure on serialize' do
      set = Set['foo']
      struct = SetPropStruct.new
      struct.set = set
      h = struct.serialize
      refute_equal(struct.set.object_id, h['set'].object_id, "`set` is the same object")
    end

    it 'does not share structure on deserialize' do
      set = Set['foo']
      h = {
        'set' => set,
      }
      struct = SetPropStruct.from_hash(h)
      refute_equal(struct.set.object_id, h['set'].object_id, "`set` is the same object")
    end
  end

  class CustomSetPropStruct
    include T::Props::Serializable
    prop :set, T::Set[CustomType]
  end

  describe 'custom set props' do
    it 'round trips' do
      array = [3]
      obj = CustomType.new
      obj.value = array
      set = Set[obj]
      struct = CustomSetPropStruct.new
      struct.set = set
      h = struct.serialize
      assert_equal(1, h['set'].length)
      assert(h['set'].include?(array))

      roundtripped = CustomSetPropStruct.from_hash(h)
      assert_equal(1, roundtripped.set.length)
      value = roundtripped.set.to_a[0].value
      assert_equal(array, value)
    end

    it 'raises serialize errors' do
      not_a_set = 1234
      struct = CustomSetPropStruct.new
      struct.instance_variable_set(:@set, not_a_set)
      e = assert_raises(TypeError) do
        struct.serialize
      end

      assert_includes(e.message, "value must be enumerable")
    end

    it 'raises deserialize errors' do
      msg_string = nil
      extra_hash = nil
      T::Configuration.soft_assert_handler = proc do |msg, extra|
        msg_string = msg
        extra_hash = extra
      end

      obj = CustomType.new
      e = assert_raises(TypeError) do
        CustomSetPropStruct.from_hash({'set' => obj})
      end

      assert_includes(e.message, "value must be enumerable")
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
    prop :deprecated_enum_of_enums, T.nilable(T.deprecated_enum([MyEnum::BAR]))
  end

  class RedundantEnumStruct < T::Struct
    prop :deprecated_enum, T.all(MyEnum, T.deprecated_enum(MyEnum.values))
  end

  describe 'enum' do
    it 'round trips' do
      s = EnumStruct.new(enum: MyEnum::FOO, deprecated_enum_of_enums: MyEnum::BAR)

      serialized = s.serialize
      assert_equal('foo', serialized['enum'])
      assert_equal('bar', serialized['deprecated_enum_of_enums'])

      roundtripped = EnumStruct.from_hash(serialized)
      assert_equal(MyEnum::FOO, roundtripped.enum)
      assert_equal(MyEnum::BAR, roundtripped.deprecated_enum_of_enums)
    end

    it 'does not break during serde when used redundantly with legacy T.deprecated_enum' do
      s = RedundantEnumStruct.new(deprecated_enum: MyEnum::FOO)
      serialized = s.serialize
      assert_equal('foo', serialized['deprecated_enum'])

      roundtripped = RedundantEnumStruct.from_hash(serialized)
      assert_equal(MyEnum::FOO, roundtripped.deprecated_enum)
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
      T::Configuration.scalar_types = T::Configuration.scalar_types.keys + [ModulePropStruct::Scalar.name]
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

  class Hashlike < Hash
    def initialize
      @called = false
      super()
    end

    def [](key)
      @called = true
      super(key)
    end

    def was_called
      @called
    end
  end

  class StructForHashlike
    include T::Props::Serializable

    prop :stringprop, String
  end

  describe 'with a custom hash-like type' do
    it 'calls the overridden aref method' do
      h = Hashlike['stringprop', 'foo']
      assert_instance_of(Hashlike, h)
      obj = StructForHashlike.from_hash(h)
      assert_equal("foo", obj.stringprop)
      assert(h.was_called)
    end
  end

  class DefaultStringProp
    include T::Props::Serializable

    prop :stringprop, String, default: "default"
  end

  describe 'with defaulted string props' do
    it 'does not share structure' do
      h = {}
      obj1 = DefaultStringProp.from_hash(h)
      obj2 = DefaultStringProp.from_hash(h)
      refute_equal(obj1.stringprop.object_id, obj2.stringprop.object_id)
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

      describe 'with custom module_name_mangler' do
        before do
          T::Configuration.module_name_mangler = ->(type) {"MANGLED::#{type}"}
        end

        after do
          T::Configuration.module_name_mangler = nil
        end

        it 'mangles the custom type names in generated code' do
          src = ComplexStruct.decorator.send(:generate_deserialize_source).to_s
          T::Props::GeneratedCodeValidation.validate_deserialize(src)
          assert_includes(src, "MANGLED::#{MySerializable}")
          assert_includes(src, "MANGLED::#{CustomType}")
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

  class StructWithFloat < T::Struct
    const :my_float, Float
  end

  it "coerces raw values to Float if the prop's type is Float" do
    swf1 = StructWithFloat.new(my_float: 1.0)

    serialized_hash = swf1.serialize
    # Ruby happens to serialize Float's with a `.0`, but JSON doesn't
    # distinguish number types, so it's equally fine to omit it, and the
    # process of serializing and deserializing is free to drop that
    # information.
    #
    # To get Ruby to omit it, we change it to an Integer before calling `.to_json`
    serialized_hash['my_float'] = 1
    json = serialized_hash.to_json

    assert_equal('{"my_float":1}', json)

    deserialized_hash = JSON.parse(json)

    assert_instance_of(Integer, deserialized_hash['my_float'])
    assert_equal(1, deserialized_hash['my_float'])

    swf2 = StructWithFloat.from_hash(deserialized_hash)

    assert_instance_of(Float, swf2.my_float)
    assert_in_delta(1.0, swf2.my_float, 0.0001)
  end

  class DynamicProps
    include T::Props::Serializable
    include T::Props::WeakConstructor

    prop :nilstring, T.nilable(String)
  end

  describe 'dynamic props' do
    it 'serializes when new props are added' do
      obj = DynamicProps.new(nilstring: 'foo')
      h = obj.serialize

      assert_equal('foo', h['nilstring'])

      DynamicProps.prop(:arrayprop, T::Array[String], default: ['default'])

      assert_raises(TypeError) do
        obj.serialize
      end

      obj.arrayprop = ['bar']
      h = obj.serialize
      assert_equal(['bar'], h['arrayprop'])
    end
  end

  class ConstDefault < T::Struct
    const :required_at_some_point, NilClass, default: nil
    const :still_required_prop, Integer
  end

  describe 'const NilClass' do
    it 'round-trip serializes' do
      h = {'still_required_prop' => 5}
      x = ConstDefault.from_hash!(h)
      assert_nil(x.required_at_some_point)
      x = x.with(still_required_prop: 6)
      assert_nil(x.required_at_some_point)
    end
  end

  class MuckAboutWithPropInternals
    include T::Props::Serializable
    include T::Props::WeakConstructor

    prop :nilstring, T.nilable(String)
  end

  # These tests are gross, but they are the only way certain exceptions are ever
  # going to be raised from the {de,}serialization generation process.
  #
  # Safe names are (currently) defined as /\A[A-Za-z_][A-Za-z0-9_-]*\z/.
  # We can't test everything outside of this set, but we can make a pass over
  # non-letter ASCII and get some coverage.
  DISALLOWED_CHARS = '!@#$%^&*()[{}]\|;:\'",<.>/?`~'.chars
  DISALLOWED_PREFIXES = DISALLOWED_CHARS + DISALLOWED_CHARS.map {|c| "a#{c}"}

  describe 'prop name safety checks' do
    it "catches when a prop name doesn't pass safe name checks" do
      DISALLOWED_PREFIXES.each do |c|
        bad_prop_name = :"#{c}nilstring"
        props = MuckAboutWithPropInternals.decorator.instance_variable_get(:@props)
        refute_nil(props)
        refute_nil(props[:nilstring])
        MuckAboutWithPropInternals.decorator.instance_variable_set(:@props, props.merge(bad_prop_name => props[:nilstring]))
        assert_raises(RuntimeError) do
          MuckAboutWithPropInternals.from_hash({})
        end
        MuckAboutWithPropInternals.decorator.instance_variable_set(:@props, props)
      end
    end

    it "catches when a hash key doesn't pass safe name checks" do
      DISALLOWED_PREFIXES.each do |c|
        ok_prop_name = :nilstring2
        bad_hash_key = "#{c}nilstring"
        props = MuckAboutWithPropInternals.decorator.instance_variable_get(:@props)
        refute_nil(props)
        refute_nil(props[:nilstring])
        bad_rules = props[:nilstring].merge(serialized_form: bad_hash_key)
        MuckAboutWithPropInternals.decorator.instance_variable_set(:@props, props.merge(ok_prop_name => bad_rules))
        assert_raises(RuntimeError) do
          MuckAboutWithPropInternals.from_hash({})
        end
        MuckAboutWithPropInternals.decorator.instance_variable_set(:@props, props)
      end
    end

    it "catches when the accessor key doesn't begin with @" do
      DISALLOWED_PREFIXES.reject {|c| c == "@"}.each do |c|
        ok_prop_name = :nilstring2
        bad_accessor_key = :"#{c}nilstring"
        props = MuckAboutWithPropInternals.decorator.instance_variable_get(:@props)
        refute_nil(props)
        refute_nil(props[:nilstring])
        bad_rules = props[:nilstring].merge(accessor_key: bad_accessor_key)
        MuckAboutWithPropInternals.decorator.instance_variable_set(:@props, props.merge(ok_prop_name => bad_rules))
        assert_raises(RuntimeError) do
          MuckAboutWithPropInternals.from_hash({})
        end
        MuckAboutWithPropInternals.decorator.instance_variable_set(:@props, props)
      end
    end

    it "catches when the accessor key doesn't pass safe name checks" do
      DISALLOWED_PREFIXES.each do |_c|
        ok_prop_name = :nilstring2
        bad_accessor_key = :"@!nilstring"
        props = MuckAboutWithPropInternals.decorator.instance_variable_get(:@props)
        refute_nil(props)
        refute_nil(props[:nilstring])
        bad_rules = props[:nilstring].merge(accessor_key: bad_accessor_key)
        MuckAboutWithPropInternals.decorator.instance_variable_set(:@props, props.merge(ok_prop_name => bad_rules))
        assert_raises(RuntimeError) do
          MuckAboutWithPropInternals.from_hash({})
        end
        MuckAboutWithPropInternals.decorator.instance_variable_set(:@props, props)
      end
    end
  end
end
