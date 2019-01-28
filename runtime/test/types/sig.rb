# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::SigTest < Critic::Unit::UnitTest
  it 'works on a class' do
    klass = Class.new do
      sig {returns(Symbol)}
      def foo
        :foo
      end
    end
    assert_equal(:foo, klass.new.foo)
  end

  it 'works on a module' do
    mod = Module.new do
      sig {returns(Symbol)}
      def self.foo
        :foo
      end
    end
    assert_equal(:foo, mod.foo)
  end

  it 'does not work an instance ' do
    klass = Class.new do
      def foo
        sig {void}
      end
    end
    e = assert_raises do
      klass.new.foo
    end
    assert_match(/undefined method `sig' for/, e.message)
  end
end
