# frozen_string_literal: true
require_relative '../../test_helper'

class Opus::Types::Test::Props::DecoratorTest < Critic::Unit::UnitTest
  def assert_prop_error(match=nil, error: ArgumentError, mixin: T::Props, &blk)
    ex = assert_raises(error) do
      Class.new do
        include mixin
        class_exec(&blk)
      end
    end
    assert_match(match, ex.message) if match
  end

  class StringArrayAndHashStruct < T::Struct
    prop :arr, T.nilable(T::Array[String])
    prop :the_hash, T.nilable(T::Hash[String, String])
  end

  class SubstructArrayAndHashStruct < T::Struct
    class Substruct < T::Struct
      prop :id, T.nilable(Integer), default: 0

      def ==(other)
        other.class <= self.class && other.id == self.id
      end
    end

    prop :arr, T.nilable(T::Array[Substruct])
    prop :the_hash, T.nilable(T::Hash[String, Substruct])
  end

  class CustomTypeArrayAndHashStruct < T::Struct
    class CustomType
      extend T::Props::CustomType

      def self.instance?(value)
        value.is_a?(String)
      end

      def self.deserialize(value)
        value.clone.freeze
      end

      def self.serialize(instance)
        instance
      end
    end

    prop :arr, T.nilable(T::Array[CustomType])
    prop :the_hash, T.nilable(T::Hash[String, CustomType])
  end

  describe 'typed arrays and hashes' do
    it 'can have string values' do
      doc = StringArrayAndHashStruct.new(
        arr: ['foo'],
        the_hash: {'foo' => 'bar'},
      )
      assert_equal({"arr" => ["foo"], "the_hash" => {"foo" => "bar"}}, doc.serialize)
      assert_equal(doc.serialize, StringArrayAndHashStruct.from_hash(doc.serialize).serialize)
    end

    it 'can have substruct values' do
      doc = SubstructArrayAndHashStruct.new(
        arr: [SubstructArrayAndHashStruct::Substruct.new(id: 1)],
        the_hash: {'foo' => SubstructArrayAndHashStruct::Substruct.new(id: 2)},
      )

      assert_equal({"arr" => [{'id' => 1}], "the_hash" => {"foo" => {'id' => 2}}}, doc.serialize)
      assert_equal(doc.serialize, SubstructArrayAndHashStruct.from_hash(doc.serialize).serialize)
    end

    it 'can have nil values for substructs' do
      doc = SubstructArrayAndHashStruct.new(
        arr: nil,
        the_hash: nil,
      )

      assert_equal({}, doc.serialize)
    end

    it 'can have custom type values' do
      CustomTypeArrayAndHashStruct.new(
        arr: [CustomTypeArrayAndHashStruct::CustomType.new],
        the_hash: {'foo' => CustomTypeArrayAndHashStruct::CustomType.new},
      )
    end

    it 'allows FixedArrays' do
      # We still need to build support for this
      skip
      Class.new do
        include T::Props
        prop :fixed_array_prop, T::Utils.coerce([String, Numeric])
      end
    end
  end

  describe 'When validating prop definitions' do
    it 'Validates prop options are symbols' do
      assert_prop_error(error: ArgumentError) do
        prop :foo, String, 'name' => 'mongoprop'
      end
    end

    it 'Validates prop options are recognize' do
      assert_prop_error(/At least one invalid prop arg/) do
        prop :foo, String, nosucharg: :llamas
      end
    end

    it 'Validates you pass a type' do
      assert_prop_error(/Invalid String literal for type constraint.*Got a String with value `goat`/, error: RuntimeError) do
        prop :foo, "goat"
      end
    end

    it 'allows plain objects' do
      Class.new do
        include T::Props
        prop :foo, Object
      end
    end

    it 'allows untyped arrays' do
      Class.new do
        include T::Props
        prop :foo, Array
      end
    end

    it 'allows untyped hashes' do
      Class.new do
        include T::Props
        prop :foo, Hash
      end
    end

    it 'disallows bang on props' do
      assert_raises(ArgumentError) do
        Class.new do
          include T::Props
          prop :do_something!, String # boom: bang
        end
      end
    end

    it 'disallows setters' do
      assert_raises(ArgumentError) do
        Class.new do
          include T::Props
          prop :foo=, String # boom: =
        end
      end
    end

    it 'disallows spaces' do
      assert_raises(ArgumentError) do
        Class.new do
          include T::Props
          prop :'foo bar', String # boom: space
        end
      end
    end
  end

  class StructHash < T::Struct
    class InnerStruct < T::Struct

    end
    prop :the_hash, T::Hash[String, InnerStruct]
  end

  describe 'validating prop values' do
    it 'validates that subdoc hashes have the correct values' do

      assert_raises(TypeError) do
        StructHash.new(the_hash: {'foo' => {}})
      end

      # no raise:
      StructHash.new(the_hash: {'foo' => StructHash::InnerStruct.new})
    end
  end

  class OptionalArrayClass
    include T::Props::Serializable
    prop :foo, T.nilable(T::Array[Integer])
  end

  describe 'optional props' do
    it 'can be used with typed arrays' do
      foo = OptionalArrayClass.props.fetch(:foo)
      assert_equal(T::Types::TypedArray, foo.fetch(:type).class, T::Types::TypedArray)
      assert_equal(Integer, foo.fetch(:array), Integer)
      assert(T::Props::Utils.optional_prop?(foo))
    end
  end

  class OptionalMigrate
    include T::Props::Serializable
    include T::Props::WeakConstructor

    prop :foo, String
    prop :bar, String, default: 'bar'
  end

  describe 'optional: false props' do
    it 'works if the field is there' do
      m = OptionalMigrate.from_hash('foo' => 'foo')
      assert_equal('foo', m.foo)
      assert_equal('bar', m.bar)
    end

    it "won't populate in from_hash" do
      e = assert_raises do
        OptionalMigrate.from_hash({})
      end
      assert_match(/Tried to deserialize a required prop from a nil value./, e.message)
    end

    it "won't serialize with a missing prop" do
      e = assert_raises do
        OptionalMigrate.new({}).serialize
      end
      assert_match(/Opus::Types::Test::Props::DecoratorTest::OptionalMigrate.foo not set/, e.message)
    end

    it "will try to alert the owner if possible" do

      found_team = nil
      T::Configuration.class_owner_finder = ->(_klass) {:some_team}
      # because `raise_nil_deserialize_error` has a final `ensure`
      # block, we're going to end up calling this twice, and only
      # once with the `project:` key set. Expressing that via
      # `.expect` here is a bit messy, so we're going to set a
      # variable if we get the assert handler called once with the
      # right project
      T::Configuration.hard_assert_handler = lambda do |_msg, kwargs|
        found_team = kwargs[:project] if kwargs.include?(:project)
      end
      OptionalMigrate.from_hash({})
      assert_equal(:some_team, found_team)
    ensure
      T::Configuration.hard_assert_handler = nil
      T::Configuration.class_owner_finder = nil

    end

  end

  class OptionalMigrate2
    include T::Props::Serializable
    include T::Props::WeakConstructor

    prop :foo, String
    prop :bar, String, default: 'bar'
  end

  describe 'optional: false props are the default' do
    it 'works if the field is there' do
      m = OptionalMigrate.from_hash('foo' => 'foo')
      assert_equal('foo', m.foo)
      assert_equal('bar', m.bar)
    end

    it "won't populate in from_hash" do
      e = assert_raises do
        OptionalMigrate.from_hash({})
      end
      assert_match(/Tried to deserialize a required prop from a nil value./, e.message)
    end

    it "won't serialize with a missing prop" do
      e = assert_raises do
        OptionalMigrate.new({}).serialize
      end
      assert_match(/Opus::Types::Test::Props::DecoratorTest::OptionalMigrate.foo not set/, e.message)
    end
  end

  class ImmutablePropStruct
    include T::Props::WeakConstructor

    prop :immutable, String, immutable: true
    const :const, String
  end

  class ConstArrayStruct < T::Struct
    const :foo, T::Array[Integer]
  end

  describe 'immutable props' do
    it 'lets setting in constructor' do
      m = ImmutablePropStruct.new(immutable: 'hello')
      assert_equal('hello', m.immutable)
    end

    it 'does not allow setting' do
      m = ImmutablePropStruct.new(immutable: 'hello')
      e = assert_raises(NoMethodError) do
        m.immutable = 'world'
      end
      assert_match(/undefined method [`']immutable='/, e.message)
    end

    it 'const creates an immutable prop' do
      assert(ImmutablePropStruct.props[:const][:immutable])
    end

    it 'can be used with typed arrays' do
      foo = ConstArrayStruct.props.fetch(:foo)
      assert_equal(T::Types::TypedArray, foo.fetch(:type).class, T::Types::TypedArray)
      assert_equal(Integer, foo.fetch(:array), Integer)
      assert(foo.fetch(:immutable))
    end

    it "validates setting 'immutable' argument when defining with 'immutable' keyword" do
      assert_prop_error(/Cannot pass 'immutable' argument/) do
        const :foo, String, immutable: false
      end
    end
  end

  # Testing the matrix in chalk/odm/doc/chalk-odm.md
  class MatrixStruct
    include T::Props::Serializable
    include T::Props::WeakConstructor

    prop :a, T.nilable(Integer), raise_on_nil_write: true
    prop :b, T.nilable(Integer)
    const :c, T.nilable(Integer), raise_on_nil_write: true
    prop :d, Integer, default: 91
    prop :e, Integer
  end

  it 'writes as expected' do
    e = assert_raises do
      MatrixStruct.new.a = nil
    end
    assert_match(/to nil/, e.message)
    m = MatrixStruct.new
    m.b = nil
    assert_nil(m.b)
    e = assert_raises do
      MatrixStruct.new.c = nil
    end
    assert_match(/undefined method [`']c='/, e.message)
    e = assert_raises do
      MatrixStruct.new.d = nil
    end
    assert_match(/to nil/, e.message)
    e = assert_raises do
      MatrixStruct.new.e = nil
    end
    assert_match(/to nil/, e.message)
  end

  it 'deserializes as expected' do
    assert_nil(MatrixStruct.from_hash("a" => nil, "e" => 5).a)
    assert_nil(MatrixStruct.from_hash("b" => nil, "e" => 5).b)
    assert_nil(MatrixStruct.from_hash("c" => nil, "e" => 5).c)
    assert_equal(91, MatrixStruct.from_hash("d" => nil, "e" => 5).d)
    e = assert_raises do
      MatrixStruct.from_hash("e" => nil)
    end
    assert_match(/Tried to deserialize a required prop from a nil value./, e.message)
  end

  it 'constructs as expected' do
    assert_nil(MatrixStruct.new.a)
    assert_nil(MatrixStruct.new.b)
    assert_nil(MatrixStruct.new.c)
    assert_equal(91, MatrixStruct.new.d)
    assert_nil(MatrixStruct.new.e)
  end

  it 'raises if the word secret appears in a prop without a sensitivity annotation' do
    e = assert_raises do
      Class.new(T::Struct) do
        prop :secret, String
      end
    end
    assert_match(/has the word 'secret' in its name/, e.message)
  end

  it 'applies the supplied sensitivity and PII handler' do

    T::Configuration.normalize_sensitivity_and_pii_handler = lambda do |meta|
      meta[:pii] = :set
      meta[:sensitivity] += 1
      meta
    end
    e = Class.new(T::Struct) do
      # needs this annotation for the `:pii` field
      def self.contains_pii?
        true
      end

      prop :foo, Integer, sensitivity: 5
    end
    assert_equal(6, e.props[:foo][:sensitivity])
    assert_equal(:set, e.props[:foo][:pii])
  ensure
    T::Configuration.normalize_sensitivity_and_pii_handler = nil

  end
end
