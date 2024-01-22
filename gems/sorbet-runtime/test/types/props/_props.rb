# frozen_string_literal: true
require_relative '../../test_helper'

class Opus::Types::Test::Props::PropsTest < Critic::Unit::UnitTest
  class MyProps
    include T::Props
    prop :name, String
    prop :foo, T::Hash[T.any(String, Symbol), Object]
  end

  def my_props_instance
    m = MyProps.new
    m.name = "Bob"
    m.foo  = {
      'age' => 7,
      'color' => 'red',
    }
    m
  end

  it 'generates accessors' do
    m = my_props_instance

    assert_equal("Bob", m.name)
    assert_equal(7, m.foo['age'])
    assert_equal('red', m.foo['color'])
  end

  it 'can validate prop value' do
    MyProps.validate_prop_value :foo, {}

    e = assert_raises(TypeError) do
      MyProps.validate_prop_value :foo, 'nope'
    end
    assert_equal(
      "Parameter 'foo': Can't set Opus::Types::Test::Props::PropsTest::MyProps.foo to \"nope\" (instance of String) - need a T::Hash[T.any(String, Symbol), Object]",
      e.message.split("\n").first,
    )

    assert_raises(RuntimeError) do
      MyProps.validate_prop_value :nope, 'does not matter'
    end
  end

  module BaseProps
    include T::Props

    prop :prop1, String
    prop :prop2, Integer, ifunset: 42
    prop :shadowed, String

    orig_verbose = $VERBOSE
    $VERBOSE = false

    def shadowed
      "I can't let you see that"
    end

    $VERBOSE = orig_verbose
  end

  class SubProps
    include BaseProps
    prop :prop3, T::Hash[T.any(String, Symbol), Object]
  end

  class OverrideSubProps
    include BaseProps
    prop :prop2, T::Array[Object], override: true
  end

  class InheritedOverrideSubProps < OverrideSubProps
  end

  class HasPropGetOverride < T::Props::Decorator
    attr_reader :field_accesses

    def prop_get(instance, prop, *)
      @field_accesses ||= []
      @field_accesses << prop
      super
    end
  end

  class UsesPropGetOverride
    include T::Props

    def self.decorator_class
      HasPropGetOverride
    end

    prop :foo, T.nilable(String)
  end

  class AddsPropsToClassWithPropGetOverride < UsesPropGetOverride
    include BaseProps
  end

  describe 'when subclassing' do
    it 'inherits properties' do
      d = SubProps.new
      d.prop1 = 'hi'
      d.prop3 = {'foo' => 'bar'}
      assert_equal('hi', d.prop1)
      assert_equal(42, d.prop2)
      assert_equal('bar', d.prop3['foo'])
    end

    it 'allows overriding props in subclasses' do
      obj = OverrideSubProps.new
      obj.prop2 = [1, 2, 3]
      assert_equal(1, obj.prop2.first)
    end

    it 'Does not clobber methods' do
      d = SubProps.new
      d.shadowed = "the darkness"
      assert_equal("I can't let you see that", d.shadowed)
    end

    it 'allows inheriting overridden props' do
      assert(InheritedOverrideSubProps.props.include?(:prop2))
    end

    it 'allows hooking prop_get' do
      d = UsesPropGetOverride.new
      d.foo = 'bar'
      d.foo

      assert_equal(
        [:foo],
        UsesPropGetOverride.decorator.field_accesses,
      )

      d = AddsPropsToClassWithPropGetOverride.new
      d.prop1 = 'bar'
      d.prop1
      assert_equal("I can't let you see that", d.shadowed)

      assert_equal(
        [:prop1],
        AddsPropsToClassWithPropGetOverride.decorator.field_accesses,
      )
    end
  end

  describe 'ifunset' do
    before do
      @doc = SubProps.new
    end

    it 'is used by getter' do
      assert_equal(42, @doc.prop2)
    end

    it 'is used by decorator#prop_get' do
      assert_equal(42, @doc.class.decorator.prop_get(@doc, :prop2))
    end

    # This distinction seems subtle and, given that it has no relationship
    # to the method names, pretty confusing. But code relies on it.
    it 'is not used by decorator#get' do
      assert_nil(@doc.class.decorator.get(@doc, :prop2))
    end
  end

  class TestRedactedProps
    include T::Props

    prop :int, Integer
    prop :str, String, redaction: :redact_digits, sensitivity: []
    prop :secret, String, redaction: [:truncate, 4], sensitivity: []
  end

  describe 'redacted props with no redaction handler' do
    it 'raises when fetching redacted values' do
      assert_raises(RuntimeError) do
        d = TestRedactedProps.new
        d.str = '12345'

        # this will raise an error without a redaction handler
        d.str_redacted
      end
    end
  end

  describe 'redacted props' do
    before do
      T::Configuration.redaction_handler = lambda do |value, redaction|
        opts = Array(redaction)
        case opts[0]
        when :redact_digits
          value.gsub(/\d/, '*')
        when :truncate
          T::Utils.string_truncate_middle(value, opts[1], 0)
        else
          value
        end
      end
    end

    after do
      T::Configuration.redaction_handler = nil
    end

    it 'gets and sets normally' do
      d = TestRedactedProps.new
      d.class.decorator.prop_set(d, :str, '12345')
      assert_equal('12345', d.str)
      d.str = '54321'
      assert_equal('54321', d.str)
    end

    it 'redacts digits' do
      d = TestRedactedProps.new
      d.str = '12345'
      assert_equal('12345', d.str)
      assert_equal('*****', d.str_redacted)
    end

    it 'handles array redaction spec' do
      d = TestRedactedProps.new
      d.secret = '1234abcd'
      assert_equal('1234abcd', d.secret)
      assert_equal('1...', d.secret_redacted)
    end
  end

  class MyTestModel
    attr_reader :id
    def initialize(id)
      @id = id
    end

    def self.load(id, extra={}, opts={})
      return nil if id.nil?
      MyTestModel.new(id)
    end
  end

  class TestForeignProps
    include T::Props

    prop :foreign1, String, foreign: -> {MyTestModel}
    prop :foreign2, T.nilable(String), foreign: -> {MyTestModel}
  end

  describe 'foreign props' do
    it 'supports nilable props' do
      obj = TestForeignProps.new

      obj.foreign1 = 'test'
      test_model = obj.foreign1_
      assert(test_model)
      assert_equal(obj.foreign1, test_model.id)

      obj.foreign2 = nil
      test_model = obj.foreign2_
      refute(test_model)

      obj.foreign2 = 'test'
      test_model = obj.foreign2_
      assert(test_model)
      assert_equal(obj.foreign2, test_model.id)
    end

    it 'disallows non-proc arguments' do
      T::Configuration.expects(:soft_assert_handler).with do |msg, _|
        msg.include?('Please use a Proc that returns a model class instead')
      end.times(1)

      Class.new(TestForeignProps) do
        prop :foreign3, String, foreign: MyTestModel
      end
    end
  end

  class MyCustomType
    extend T::Props::CustomType

    attr_accessor :value

    def initialize(value)
      @value = value
    end

    def self.deserialize(value)
      result = new
      result.value = value.clone.freeze
      result
    end

    def self.serialize(instance)
      instance.value
    end
  end

  class TestCustomProps
    include T::Props

    prop :custom, MyCustomType
    prop :nilable_custom, T.nilable(MyCustomType)
    prop :collection_custom, T::Array[MyCustomType]
  end

  describe 'custom types' do
    before do
      @obj = TestCustomProps.new
    end

    it 'work when plain' do
      @obj.custom = MyCustomType.new("foo")
      assert_equal("foo", @obj.custom.value)
    end

    it 'work when nilable' do
      @obj.nilable_custom = nil
      assert_nil(@obj.nilable_custom)

      @obj.nilable_custom = MyCustomType.new("foo")
      assert_equal("foo", @obj.nilable_custom.value)
    end

    it 'work as collection element' do
      @obj.collection_custom = []
      assert_empty(@obj.collection_custom)

      @obj.collection_custom = [MyCustomType.new("foo")]
      assert_equal(["foo"], @obj.collection_custom.map(&:value))
    end
  end

  class TestUntyped
    include T::Props

    prop :untyped, T.untyped
  end

  describe 'untyped' do
    it 'has working accessors' do
      obj = TestUntyped.new
      obj.untyped = nil
      assert_nil(obj.untyped)
      obj.untyped = 'foo'
      assert_equal('foo', obj.untyped)
    end
  end

  class TypeValidating
    include T::Props
    include T::Props::TypeValidation
  end

  describe 'type validation' do
    it 'bans plain Object' do
      assert_raises(T::Props::TypeValidation::UnderspecifiedType) do
        Class.new(TypeValidating) do
          prop :object, Object
        end
      end
    end

    it 'allows with DEPRECATED_underspecified_type' do
      c = Class.new(TypeValidating) do
        prop :object, Object, DEPRECATED_underspecified_type: true
      end
      o = c.new
      o.object = 1
      assert_equal(1, o.object)
    end

    it 'allows T.all' do
      c = Class.new(TypeValidating) do
        prop :deprecated_intersection, T.all(Object, T.deprecated_enum(["foo"]))
      end
      o = c.new
      o.deprecated_intersection = "foo"
      assert_equal("foo", o.deprecated_intersection)
    end
  end

  describe 'override checking' do
    class OverrideProps
      include T::Props
      prop :a, String
    end

    it 'errors if a prop is overridden without override => true' do
      error = assert_raises(ArgumentError) do
        class OverrideProps1 < OverrideProps
          prop :a, Integer
        end
      end

      assert(error.message.include?("Attempted to redefine prop :a on class Opus::Types::Test::Props::PropsTest::OverrideProps1 that's already defined without specifying :override => true"))
    end

    it 'allows overriding with override => true' do
      class OverrideProps2 < OverrideProps
        prop :a, Integer, override: true
      end
    end

    it 'errors if a prop has override => true but does not exist' do
      error = assert_raises(ArgumentError) do
        class OverrideProps3 < OverrideProps
          prop :b, Integer, override: true
        end
      end

      assert(error.message.include?("Attempted to override a prop :b on class Opus::Types::Test::Props::PropsTest::OverrideProps3 that doesn't already exist"))
    end
  end
end
