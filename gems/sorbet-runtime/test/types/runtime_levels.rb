# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class RuntimeLevelsTest < Critic::Unit::UnitTest
    describe 'set_enable_checking_in_tests_from_environment' do
      before do
        @orig_wrapped_tests_with_validation = T::Private::RuntimeLevels.instance_variable_get(:@wrapped_tests_with_validation)
        @orig_check_tests = T::Private::RuntimeLevels.instance_variable_get(:@check_tests)
        @orig_sorbet_runtime_enable_checking_in_tests = ENV['SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS']

        # Within these specs pretend we haven't yet read this value and checked_tests is false
        T::Private::RuntimeLevels.instance_variable_set(:@wrapped_tests_with_validation, false)
        T::Private::RuntimeLevels.instance_variable_set(:@check_tests, false)
      end

      after do
        T::Private::RuntimeLevels.instance_variable_set(:@check_tests, @orig_check_tests)
        T::Private::RuntimeLevels.instance_variable_set(:@wrapped_tests_with_validation, @orig_wrapped_tests_with_validation)
        ENV['SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS'] = @orig_sorbet_runtime_enable_checking_in_tests
      end

      describe 'when SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS env variable is not set' do
        before do
          ENV['SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS'] = nil
        end

        it 'does not change check_tests' do
          # Reaching into a private method for testing purposes
          T::Private::RuntimeLevels.send(:set_enable_checking_in_tests_from_environment)

          assert_equal(T::Private::RuntimeLevels.check_tests?, false)
        end
      end

      describe 'when SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS env variable is set' do
        before do
          ENV['SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS'] = '1'
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
        @orig_sorbet_runtime_default_checked_level = ENV['SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL']

        # Within these specs pretend we haven't yet read this value and default_checked_level is always
        T::Private::RuntimeLevels.instance_variable_set(:@has_read_default_checked_level, false)
        T::Private::RuntimeLevels.instance_variable_set(:@default_checked_level, :always)
      end

      after do
        T::Private::RuntimeLevels.instance_variable_set(:@default_checked_level, @orig_default_checked_level)
        T::Private::RuntimeLevels.instance_variable_set(:@has_read_default_checked_level, @orig_has_read_default_checked_level)
        ENV['SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL'] = @orig_sorbet_runtime_default_checked_level
      end

      describe 'when SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL env variable is not set' do
        before do
          ENV['SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL'] = nil
        end

        it 'does not change default_typed_level' do
          # Reaching into a private method for testing purposes
          T::Private::RuntimeLevels.send(:set_default_checked_level_from_environment)

          assert_equal(T::Private::RuntimeLevels.default_checked_level, :always)
        end
      end

      describe 'when SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL env variable is set' do
        before do
          ENV['SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL'] = 'never'
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
