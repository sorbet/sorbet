# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class MustBecauseTest < Critic::Unit::UnitTest
    EXAMPLE_REASON = 'some_must_because_reason'

    it 'allows non-nil' do
      assert_equal(:a, T.must_because(:a) {EXAMPLE_REASON})
      assert_equal(0, T.must_because(0) {EXAMPLE_REASON})
      assert_equal("", T.must_because("") {EXAMPLE_REASON})
      assert_equal(false, T.must_because(false) {EXAMPLE_REASON})
    end

    it 'disallows nil' do
      e = assert_raises(TypeError) do
        T.must_because(nil) {EXAMPLE_REASON}
      end

      assert_equal("Unexpected `nil` because #{EXAMPLE_REASON}", e.message)
    end

    it 'does not calculate the reason unless nil is passed' do
      T.must_because(:a) do
        raise('reason block should not have been called')
      end
    end
  end
end
