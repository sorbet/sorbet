# frozen_string_literal: true
# typed: ignore
require_relative '../test_helper'

class Opus::Types::Test::SigTest < Critic::Unit::UnitTest
  it 'works on a class' do
    klass = Class.new do
      extend T::Sig
      sig { returns(Symbol) }
      def foo
        :foo
      end
    end
    assert_equal(:foo, klass.new.foo)
  end

  it 'works on a module' do
    mod = Module.new do
      extend T::Sig
      sig { returns(Symbol) }
      def self.foo
        :foo
      end
    end
    assert_equal(:foo, mod.foo)
  end

  it 'does not work an instance ' do
    klass = Class.new do
      extend T::Sig
      def foo
        sig { void }
      end
    end
    e = assert_raises do
      klass.new.foo
    end
    assert_match(/undefined method [`']sig' for/, e.message)
  end

  # Enable $VERBOSE and redirect stderr to a string for the duration of the
  # passed block.
  def fake_stderr_with_warnings
    original_stderr = $stderr
    original_verbose = $VERBOSE
    $stderr = StringIO.new
    $VERBOSE = true
    yield
    $stderr.string
  ensure
    $stderr = original_stderr
    $VERBOSE = original_verbose
  end

  it 'does not emit warnings when overriding methods' do
    output = fake_stderr_with_warnings do
      Class.new do
        extend T::Sig
        sig { void }
        def foo; end
      end
    end

    assert_equal(output, '')
  end

  it 'stores sig keys without copying them' do
    klass = Class.new do
      extend T::Sig
      sig { returns(Symbol) }
      def foo
        :foo
      end
    end

    key = T::Private::Methods.send(:method_owner_and_name_to_key, klass, :foo)
    assert_predicate(key, :frozen?)

    sig_wrappers = T::Private::Methods.instance_variable_get(:@sig_wrappers)
    stored_key = sig_wrappers.keys.find { |k| k == key }
    refute_nil(stored_key)
    # The key captured by the method wrapper and the key stored in the Hash
    # should be the same object, not a frozen copy made by Hash#[]=.
    sig_block = sig_wrappers.fetch(key)
    assert_same(stored_key, sig_block.binding.local_variable_get(:key))

    assert_equal(:foo, klass.new.foo)
  end
end
