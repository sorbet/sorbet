# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class ReturnsTest < Critic::Unit::UnitTest
    class TestReturns
      extend T::Sig
      extend T::Generic

      sig {returns(T.noreturn)}
      def self.thrower
        raise "thrower"
      end

      sig {returns(T.noreturn)}
      def self.notthrower
        5
      end

      sig {void}
      def self.voider
        "voider"
      end

      sig {void}
      def initialize; end
    end

    it 'works with throwing methods' do
      e = assert_raises(RuntimeError) do
        TestReturns.thrower
      end
      assert_equal("thrower", e.message)
    end

    it 'fails with not throwing methods' do
      e = assert_raises(TypeError) do
        TestReturns.notthrower
      end
      assert_match(/Return value: Expected type T.noreturn, got type Integer/, e.message)
    end

    it 'nils the return for void' do
      assert_equal(T::Private::Types::Void::VOID, TestReturns.voider)
    end

    it 'can mark constructors void' do
      assert_equal(true, TestReturns.new.is_a?(TestReturns))
    end
  end
end
