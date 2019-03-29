# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class MustTest < Critic::Unit::UnitTest
    it 'allows non-nil' do
      assert_equal(:a, T.must(:a))
      assert_equal(0, T.must(0))
      assert_equal("", T.must(""))
      assert_equal(false, T.must(false))
    end

    it 'disallows nil' do
      e = assert_raises(TypeError) do
        T.must(nil)
      end
      assert_equal('Passed `nil` into T.must', e.message)
    end

    it 'takes a custom message' do
      e = assert_raises(TypeError) do
        T.must(nil, "booo")
      end
      assert_equal('booo', e.message)
    end

    it 'does not allow custom classes' do
      e = assert_raises(TypeError) do
        T.must(:a, RuntimeError.new('hi'))
      end
      assert_equal('T.must expects a string as second argument', e.message)
    end
  end
end
