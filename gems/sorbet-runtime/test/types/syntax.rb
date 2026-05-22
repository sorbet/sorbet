# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::SyntaxTest < Critic::Unit::UnitTest
  it 'works on a class' do
    klass = Class.new do
      extend T::Syntax
      sig { returns(Symbol) }
      def foo
        :foo
      end
    end
    assert_equal(:foo, klass.new.foo)
  end

  it 'works on a module' do
    mod = Module.new do
      extend T::Syntax
      sig { returns(Symbol) }
      def self.foo
        :foo
      end
    end
    assert_equal(:foo, mod.foo)
  end

  it 'does not work an instance ' do
    klass = Class.new do
      extend T::Syntax
      def foo
        sig { void }
      end
    end
    e = assert_raises do
      klass.new.foo
    end
    assert_match(/undefined method [`']sig' for/, e.message)
  end

  it 'works for abstract+override' do
    parent = Class.new do
      extend T::Syntax
      abstract!
      sig { returns(Integer) }
      abstract def foo; end
    end

    child = Class.new(parent) do
      sig { returns(String) }
      override def foo; end
    end

    e = assert_raises do
      child.new.foo
    end
    assert_match(/The types must be covariant/, e.message)
  end

  # - `abstract`/`override` with `sig`
  # - `interface!` with `abstract`
end
