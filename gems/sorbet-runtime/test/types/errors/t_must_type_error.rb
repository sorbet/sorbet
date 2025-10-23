# frozen_string_literal: true
require_relative '../../test_helper'

class Opus::Types::Test::Errors
  class TMustTypeErrorTest < Critic::Unit::UnitTest
    it 'includes a special snippet' do
      ex = assert_raises(T::MustTypeError) do
        T.must(nil)
      end

      err_lines = ex.message.split("\n")
      assert_equal(err_lines.fetch(0), "Passed `nil` into T.must")
      assert_equal(err_lines.fetch(1), "")
      assert_equal(err_lines.fetch(2), "        T.must(nil)")
      assert_equal(err_lines.fetch(3), "               ^^^")
    end

    it "points to the right place when there's more than one T.must on the line" do
      ex = assert_raises(T::MustTypeError) do
        T.must(T.must(nil))
      end

      err_lines = ex.message.split("\n")
      assert_equal(err_lines.fetch(0), "Passed `nil` into T.must")
      assert_equal(err_lines.fetch(1), "")
      assert_equal(err_lines.fetch(2), "        T.must(T.must(nil))")
      assert_equal(err_lines.fetch(3), "                      ^^^")
    end

    it 'does not raise an ArgumentError if called from an eval' do
      ex = assert_raises(T::MustTypeError) do
        eval('T.must(nil)')
      end

      # Also does not return any fancy error message snippet
      assert_equal("Passed `nil` into T.must", ex.message)
    end
  end
end
