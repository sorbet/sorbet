# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::SigTest < Critic::Unit::UnitTest
  it 'works on a class' do
    klass = Class.new do
      extend T::Sig
      sig {returns(Symbol)}
      def foo
        :foo
      end
    end
    assert_equal(:foo, klass.new.foo)
  end

  it 'works on a module' do
    mod = Module.new do
      extend T::Sig
      sig {returns(Symbol)}
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
        sig {void}
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
        sig {void}
        def foo; end
      end
    end

    assert_equal(output, '')
  end
end
