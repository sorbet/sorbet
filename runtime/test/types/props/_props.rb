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

  module BaseProps
    include T::Props

    prop :prop1, String
    prop :prop2, Integer, ifunset: 42
    prop :shadowed, String

    def shadowed
      "I can't let you see that"
    end
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
  end

  class TestRedactedProps
    include T::Props

    prop :int, Integer
    prop :str, String, redaction: :redact_digits, sensitivity: []
    prop :secret, String, redaction: [:truncate, 4], sensitivity: []
  end

  describe 'redacted props' do
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
      assert_equal('1234...', d.secret_redacted)
    end
  end

  class MyTestModel
    attr_reader :id
    def initialize(id)
      @id = id
    end

    def self.load(id, extra={}, opts={}) # rubocop:disable PrisonGuard/BanBuiltinMethodOverride
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
  end

  class BionicleEnum < Opus::Enum
    GALI = new
    POHATU = new
    ONUA = new
  end

  describe 'deep_clone_object utils method treats Opus::Enum as basic type' do
    it 'Results in same object_id for enum' do
      first = BionicleEnum::GALI
      cloned = T::Props::Utils.deep_clone_object(BionicleEnum::GALI)
      assert_equal(cloned, first)
      assert_equal(cloned.object_id, first.object_id)
    end

    it 'Results in same object if Enum is a nested object' do
      mapping = {BionicleEnum::ONUA => BionicleEnum::GALI, "foo" => {BionicleEnum::POHATU => "a toa"}, "onua" => BionicleEnum::ONUA}
      cloned = T::Props::Utils.deep_clone_object(mapping)
      assert_equal(cloned, mapping)
      assert_equal(cloned["onua"].object_id, mapping["onua"].object_id)
      assert_equal(cloned["foo"], mapping["foo"])
      assert_equal(cloned[BionicleEnum::ONUA], mapping[BionicleEnum::ONUA])
      assert_equal(cloned.keys, mapping.keys)
    end
  end
end
