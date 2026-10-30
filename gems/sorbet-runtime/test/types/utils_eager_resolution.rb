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

    it 'build_all_types builds the lazy fields of every type object' do
      fixture = "#{__dir__}/fixtures/build_all_types.rb"
      result, status = Open3.capture2e("ruby", fixture)
      assert(status.success?, "fixture failed (exit #{status.exitstatus}): #{result}")
      assert_equal("PASS\n", result)
    end

    it 'every T::Types::Base subclass implements build_lazy_fields' do
      # Anonymous subclasses are throwaways from other tests.
      missing = ObjectSpace.each_object(Class).select do |klass|
        klass < T::Types::Base && !klass.name.nil? &&
          klass.instance_method(:build_lazy_fields).owner.equal?(T::Types::Base)
      end
      assert_empty(missing.map(&:name).sort, "build_all_types would raise on these types")
    end
  end
end
