# frozen_string_literal: true

require_relative '../test_helper'

module Opus::Types::Test
  class ProfileTest < Critic::Unit::UnitTest
    it 'does not return NaN from typecheck_duration_estimate after reset' do
      T::Profile.reset
      assert(!T::Profile.typecheck_duration_estimate.nan?)
    end
  end
end
