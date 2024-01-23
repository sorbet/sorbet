# frozen_string_literal: true
require_relative '../../test_helper'

class Opus::Types::Test::Props::OptionalTest < Critic::Unit::UnitTest
  class DefaultsStruct
    include T::Props::Serializable
    include T::Props::WeakConstructor

    def self.prop2_source=(val)
      @prop2_source = val
    end

    def self.prop2_source
      @prop2_source
    end

    prop :prop1, T.nilable(String), default: "this is prop 1"
    prop :prop2, T.nilable(Integer), factory: -> {DefaultsStruct.prop2_source += 1}
    prop :trueprop, T::Boolean, default: true
    prop :falseprop, T::Boolean, default: false
    prop :default_integer, T.nilable(String), default: 123
    prop :default_array, T.nilable(String), default: []
    prop :default_mutable_string, T.nilable(String), default: String.new('hello') # rubocop:disable Performance/UnfreezeString
  end

  it 'uses default and factory props' do
    DefaultsStruct.prop2_source = 50
    m1 = DefaultsStruct.new
    m2 = DefaultsStruct.new

    assert_equal("this is prop 1", m1.prop1)
    assert_equal("this is prop 1", m2.prop1)
    assert_equal(51, m1.prop2)
    assert_equal(52, m2.prop2)
    assert_equal(52, DefaultsStruct.prop2_source)
  end

  it 'shares structure where appropriate' do
    DefaultsStruct.prop2_source = 0
    m1 = DefaultsStruct.new
    m2 = DefaultsStruct.new

    # Same object
    assert_same(m1.prop1, m2.prop1)
    assert_same(m1.trueprop, m2.trueprop)
    assert_same(m1.falseprop, m2.falseprop)
    assert_same(m1.default_integer, m2.default_integer)

    # Equal, but different object
    refute_same(m1.default_array, m2.default_array)
    refute_same(m1.default_mutable_string, m2.default_mutable_string)
    assert_equal(m1.default_array, m2.default_array)
    assert_equal(m1.default_mutable_string, m2.default_mutable_string)
  end

  it 'overrides defaults' do
    DefaultsStruct.prop2_source = 0
    m = DefaultsStruct.new(prop1: 'foo',
                           prop2: 99)
    assert_equal('foo', m.prop1)
    assert_equal(99, m.prop2)
    assert_equal(0, DefaultsStruct.prop2_source)
  end

  it 'overrides defaults even if value is nil' do
    m = DefaultsStruct.new(prop1: nil, prop2: nil)
    assert_nil(m.prop1)
    assert_nil(m.prop2)
  end
end
