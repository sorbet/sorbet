# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class RuntimeLevelsTest < Critic::Unit::UnitTest
    describe 'set_enable_checking_in_tests_from_environment' do
      before do
        @orig_wrapped_tests_with_validation = T::Private::RuntimeLevels.instance_variable_get(:@wrapped_tests_with_validation)
        @orig_check_tests = T::Private::RuntimeLevels.instance_variable_get(:@check_tests)

        # Within these specs pretend we haven't yet read this value
        T::Private::RuntimeLevels.instance_variable_set(:@wrapped_tests_with_validation, false)
      end

      after do
        T::Private::RuntimeLevels.instance_variable_set(:@check_tests, @orig_check_tests)
        T::Private::RuntimeLevels.instance_variable_set(:@wrapped_tests_with_validation, @orig_wrapped_tests_with_validation)
      end

      it 'does not change check_tests' do
        # Reaching into a private method for testing purposes
        T::Private::RuntimeLevels.send(:set_enable_checking_in_tests_from_environment)

        assert_equal(T::Private::RuntimeLevels.check_tests?, false)
      end

      describe 'when SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS env variable is set' do
        before do
          @orig_sorbet_runtime_enable_checking_in_tests = ENV['SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS']
          ENV['SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS'] = '1'
        end

        after do
          ENV['SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS'] = @orig_sorbet_runtime_enable_checking_in_tests
        end

        it 'updates check_tests' do
          # Reaching into a private method for testing purposes
          T::Private::RuntimeLevels.send(:set_enable_checking_in_tests_from_environment)

          assert_equal(T::Private::RuntimeLevels.check_tests?, true)
        end
      end
    end

    describe 'set_default_checked_level_from_environment' do
      before do
        @orig_has_read_default_checked_level = T::Private::RuntimeLevels.instance_variable_get(:@has_read_default_checked_level)
        @orig_default_checked_level = T::Private::RuntimeLevels.instance_variable_get(:@default_checked_level)

        # Within these specs pretend we haven't yet read this value
        T::Private::RuntimeLevels.instance_variable_set(:@has_read_default_checked_level, false)
      end

      after do
        T::Private::RuntimeLevels.instance_variable_set(:@default_checked_level, @orig_default_checked_level)
        T::Private::RuntimeLevels.instance_variable_set(:@has_read_default_checked_level, @orig_has_read_default_checked_level)
      end

      it 'does not change default_typed_level' do
        # Reaching into a private method for testing purposes
        T::Private::RuntimeLevels.send(:set_default_checked_level_from_environment)

        assert_equal(T::Private::RuntimeLevels.default_checked_level, :always)
      end

      describe 'when SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL env variable is set' do
        before do
          @orig_sorbet_runtime_default_checked_level = ENV['SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL']
          ENV['SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL'] = 'never'
        end

        after do
          ENV['SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL'] = @orig_sorbet_runtime_default_checked_level
        end

        it 'updates default_typed_level' do
          # Reaching into a private method for testing purposes
          T::Private::RuntimeLevels.send(:set_default_checked_level_from_environment)

          assert_equal(T::Private::RuntimeLevels.default_checked_level, :never)
        end
      end
    end
  end
end
