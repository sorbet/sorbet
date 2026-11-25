# frozen_string_literal: true
# typed: ignore
require_relative '../test_helper'

module Opus::Types::Test
  class TypeAliasCheckedTest < Critic::Unit::UnitTest
    Expected = T.type_alias { Integer }
    T1 = T.type_alias { Integer }.checked(:always)
    T2 = T.type_alias { Integer }.checked(:never)

    it 'name is not affected by `.checked`' do
      assert_equal(Expected.name, T1.name)
      assert_equal(Expected.name, T2.name)
    end

    describe '.checked(:always)' do
      it 'validates types' do
        type_alias = T.type_alias { Integer }.checked(:always)
        assert_nil(type_alias.error_message_for_obj(1))
        assert_match(/Expected type Integer, got type String/, type_alias.error_message_for_obj("hello"))
      end
    end

    describe '.checked(:never)' do
      it 'valid? returns true for any object' do
        type_alias = T.type_alias { Integer }.checked(:never)
        assert(type_alias.valid?(1))
        assert(type_alias.valid?("hello"))
        assert(type_alias.valid?(nil))
      end
    end

    it '.checked(:tests) behavior, load order, and override checking' do
      result, _status = Open3.capture2e("ruby", "test/types/fixtures/type_alias_checked_tests.rb")
      flunk(result) unless result.empty?
    end

    it 'enable_checking_for_sigs_marked_checked_tests raises if called too late' do
      result, _status = Open3.capture2e("ruby", "test/types/fixtures/type_alias_checked_tests_trapdoor.rb")
      flunk(result) unless result.empty?
    end

    it '.checked called twice raises' do
      type_alias = T.type_alias { Integer }.checked(:always)
      err = assert_raises(RuntimeError) do
        type_alias.checked(:never)
      end
      assert_includes(err.message, "can't call .checked multiple times")
    end

    it 'invalid level raises ArgumentError' do
      type_alias = T.type_alias { Integer }
      assert_raises(ArgumentError) { type_alias.checked(:foo) }
      assert_raises(ArgumentError) { type_alias.checked(true) }
      assert_raises(ArgumentError) { type_alias.checked(false) }
    end

    describe 'default_checked_level integration' do
      before do
        @orig_default_checked_level = T::Private::RuntimeLevels.instance_variable_get(:@default_checked_level)
        @orig_has_read_default_checked_level = T::Private::RuntimeLevels.instance_variable_get(:@has_read_default_checked_level)
      end

      after do
        T::Private::RuntimeLevels.instance_variable_set(:@default_checked_level, @orig_default_checked_level)
        T::Private::RuntimeLevels.instance_variable_set(:@has_read_default_checked_level, @orig_has_read_default_checked_level)
      end

      it 'no .checked + default :never — skips validation' do
        T::Private::RuntimeLevels.instance_variable_set(:@default_checked_level, :never)
        T::Private::RuntimeLevels.instance_variable_set(:@has_read_default_checked_level, false)
        type_alias = T.type_alias { Integer }
        assert(type_alias.valid?("hello"))
      end

      it 'explicit .checked(:always) overrides default :never' do
        T::Private::RuntimeLevels.instance_variable_set(:@default_checked_level, :never)
        T::Private::RuntimeLevels.instance_variable_set(:@has_read_default_checked_level, false)
        type_alias = T.type_alias { Integer }.checked(:always)
        refute(type_alias.valid?("hello"))
      end
    end

    describe 'override checking uses the real type (aliased_type)' do
      it 'subtype_of? uses aliased_type even when checked(:never)' do
        int_alias = T.type_alias { Integer }.checked(:never)
        numeric_alias = T.type_alias { Numeric }.checked(:never)

        assert(int_alias.subtype_of?(numeric_alias))
        refute(numeric_alias.subtype_of?(int_alias))
      end

      it 'override checking passes for compatible types with checked(:never)' do
        parent = Class.new do
          extend T::Sig

          sig { overridable.params(x: Numeric).returns(Integer) }
          def foo(x); 0; end
        end

        int_alias = T.type_alias { Integer }.checked(:never)
        numeric_alias = T.type_alias { Numeric }.checked(:never)

        child = Class.new(parent) do
          sig { override.params(x: numeric_alias).returns(int_alias) }
          def foo(x); 0; end
        end

        assert_equal(0, child.new.foo(1))
      end

      it 'override checking rejects incompatible types even with checked(:never)' do
        parent = Class.new do
          extend T::Sig

          sig { overridable.params(x: Integer).void }
          def foo(x); end
        end

        child = Class.new(parent) do
          string_alias = T.type_alias { String }.checked(:never)
          sig { override.params(x: string_alias).void }
          def foo(x); end
        end

        err = assert_raises(RuntimeError) do
          child.new.foo("hello")
        end
        assert_match(/Incompatible type for arg/, err.message)
      end
    end
  end
end
