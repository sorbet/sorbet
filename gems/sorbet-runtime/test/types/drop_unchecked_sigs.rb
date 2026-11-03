# frozen_string_literal: true
# typed: ignore
require_relative '../test_helper'

module Opus::Types::Test
  class DropUncheckedSigsTest < Critic::Unit::UnitTest
    it 'drops the unchecked sigs, and keeps the methods and their aliases working' do
      fixture = "#{__dir__}/fixtures/drop_unchecked_sigs.rb"
      result, status = Open3.capture2e("ruby", fixture)
      assert(status.success?, "fixture failed (exit #{status.exitstatus}): #{result}")
      assert_equal("PASS\n", result)
    end

    it 'keeps the `.checked(:tests)` sigs in a test environment' do
      fixture = "#{__dir__}/fixtures/drop_unchecked_sigs_checked_tests.rb"
      result, status = Open3.capture2e("ruby", fixture)
      assert(status.success?, "fixture failed (exit #{status.exitstatus}): #{result}")
      assert_equal("PASS\n", result)
    end
  end
end
