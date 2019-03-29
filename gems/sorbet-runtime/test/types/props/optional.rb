# frozen_string_literal: true
require_relative '../../test_helper'

class Opus::Types::Test::Props::OptionalTest < Critic::Unit::UnitTest
  class DefaultsStruct
    include T::Props::Serializable
    include T::Props::WeakConstructor

    def self.prop2_source=(val)
      @@prop2_source = val
    end

    def self.prop2_source
      @@prop2_source
    end

    prop :prop1, T.nilable(String), default: "this is prop 1"
    prop :prop2, T.nilable(Integer), factory: -> {DefaultsStruct.prop2_source += 1}
    prop     :trueprop, Boolean, default: true
    prop     :falseprop, Boolean, default: false
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
