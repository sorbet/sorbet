# frozen_string_literal: true
require_relative '../../../test_helper'

class Opus::Types::Test::Props::Private::SetterFactoryTest < Critic::Unit::UnitTest
  class TestSetValidate
    include T::Props
    include T::Props::WeakConstructor

    prop :validated, Integer, setter_validate: ->(_prop, _value) {raise Error.new 'invalid'}
    prop :nilable_validated, T.nilable(Integer), setter_validate: ->(_prop, _value) {raise Error.new 'invalid'}
    prop :unvalidated, Integer, setter_validate: ->(prop, _value) {raise Error.new 'bad prop' unless prop == :unvalidated}
    prop :untyped, T.untyped, setter_validate: ->(_prop, _value) {raise Error.new 'invalid'}

  end

  describe 'setter_validate' do
    it 'runs when setting' do
      obj = TestSetValidate.new
      ex = assert_raises {obj.validated = 5}
      assert_equal('invalid', ex.message)
    end

    it "doesn't break set" do
      obj = TestSetValidate.new(unvalidated: 3)
      assert_equal(3, obj.unvalidated)
    end

    it 'runs when constructing' do
      ex = assert_raises {TestSetValidate.new(validated: 5)}
      assert_equal('invalid', ex.message)
    end

    it 'does not run when a nilable is nil' do
      TestSetValidate.new(nilable_validated: nil)
    end

    it 'runs when a nilable is non-nil' do
      ex = assert_raises {TestSetValidate.new(nilable_validated: 5)}
      assert_equal('invalid', ex.message)
    end

    it 'runs when validate_prop_value is called' do
      ex = assert_raises {TestSetValidate.validate_prop_value(:validated, 5)}
      assert_equal('invalid', ex.message)
    end

    it 'runs on T.untyped' do
      obj = TestSetValidate.new
      ex = assert_raises {obj.untyped = 5}
      assert_equal('invalid', ex.message)

      ex = assert_raises {TestSetValidate.new(untyped: 5)}
      assert_equal('invalid', ex.message)
    end

    it 'does not run when validate_prop_value is called when a nilable is nil' do
      TestSetValidate.validate_prop_value(:nilable_validated, nil)
    end
  end

end
