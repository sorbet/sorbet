# frozen_string_literal: true
# typed: ignore
require_relative '../test_helper'

module Opus::Types::Test
  class UtilsEagerResolutionTest < Critic::Unit::UnitTest
    it 'run_all_type_alias_blocks forces effective_aliased_type on all TypeAlias objects' do
      fixture = "#{__dir__}/fixtures/resolve_all_type_aliases.rb"
      result, status = Open3.capture2e("ruby", fixture)
      assert(status.success?, "fixture failed (exit #{status.exitstatus}): #{result}")
      assert_equal("PASS\n", result)
    end

    it 'eagerly_define_all_lazy_props_methods! defines all lazy prop methods' do
      fixture = "#{__dir__}/fixtures/eagerly_define_all_lazy_props_methods.rb"
      result, status = Open3.capture2e("ruby", fixture)
      assert(status.success?, "fixture failed (exit #{status.exitstatus}): #{result}")
      assert_equal("PASS\n", result)
    end
  end
end
