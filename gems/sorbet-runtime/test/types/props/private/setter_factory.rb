# frozen_string_literal: true
# typed: ignore
require_relative '../../../test_helper'

class Opus::Types::Test::Props::Private::SetterFactoryTest < Critic::Unit::UnitTest
  class TestSetValidate
    include T::Props
    include T::Props::WeakConstructor

    prop :validated, Integer, setter_validate: ->(_prop, _value) { raise Error.new 'invalid' }
    prop :nilable_validated, T.nilable(Integer), setter_validate: ->(_prop, _value) { raise Error.new 'invalid' }
    prop :unvalidated, Integer, setter_validate: ->(prop, _value) { raise Error.new 'bad prop' unless prop == :unvalidated }
    prop :untyped, T.untyped, setter_validate: ->(_prop, _value) { raise Error.new 'invalid' }

  end

  describe 'setter_validate' do
    it 'runs when setting' do
      obj = TestSetValidate.new
      ex = assert_raises { obj.validated = 5 }
      assert_equal('invalid', ex.message)
    end

    it "doesn't break set" do
      obj = TestSetValidate.new(unvalidated: 3)
      assert_equal(3, obj.unvalidated)
    end

    it 'runs when constructing' do
      ex = assert_raises { TestSetValidate.new(validated: 5) }
      assert_equal('invalid', ex.message)
    end

    it 'does not run when a nilable is nil' do
      TestSetValidate.new(nilable_validated: nil)
    end

    it 'runs when a nilable is non-nil' do
      ex = assert_raises { TestSetValidate.new(nilable_validated: 5) }
      assert_equal('invalid', ex.message)
    end

    it 'runs when validate_prop_value is called' do
      ex = assert_raises { TestSetValidate.validate_prop_value(:validated, 5) }
      assert_equal('invalid', ex.message)
    end

    it 'runs on T.untyped' do
      obj = TestSetValidate.new
      ex = assert_raises { obj.untyped = 5 }
      assert_equal('invalid', ex.message)

      ex = assert_raises { TestSetValidate.new(untyped: 5) }
      assert_equal('invalid', ex.message)
    end

    it 'does not run when validate_prop_value is called when a nilable is nil' do
      TestSetValidate.validate_prop_value(:nilable_validated, nil)
    end
  end

  class TestSoftError
    include T::Props
    include T::Props::WeakConstructor

    prop :i, Integer
    prop :ni, T.nilable(Integer)
  end

  describe 'when call_validation_error_handler does not raise' do
    # Pins the deliberate set-after-soft-error behavior across every setter
    # entry point: the value must still be set when the handler returns.
    before do
      T::Configuration.call_validation_error_handler = ->(_signature, _opts) {}
    end

    after do
      T::Configuration.call_validation_error_handler = nil
    end

    it 'still sets the value from the constructor' do
      obj = TestSoftError.new(i: 'nope', ni: 'also nope')
      assert_equal('nope', obj.i)
      assert_equal('also nope', obj.ni)
    end

    it 'still sets the value via Decorator#prop_set' do
      obj = TestSoftError.new
      TestSoftError.decorator.prop_set(obj, :i, 'nope')
      assert_equal('nope', obj.i)
      TestSoftError.decorator.prop_set(obj, :ni, 'also nope')
      assert_equal('also nope', obj.ni)
    end

    it 'still sets the value via the generated setter' do
      obj = TestSoftError.new
      obj.i = 'nope'
      assert_equal('nope', obj.i)
      obj.ni = 'also nope'
      assert_equal('also nope', obj.ni)
    end
  end
end
