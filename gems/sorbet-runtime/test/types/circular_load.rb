# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class CircularLoadTest < Critic::Unit::UnitTest
    it "allows a method to be called while its sig is loading" do
      require_relative './fixtures/circular_load/_impl'

      Fixtures::CircularLoad::Child1.new

      assert_equal(
        [Fixtures::CircularLoad::Child2, Fixtures::CircularLoad::Child1],
        Fixtures::CircularLoad::Parent.foo_invocations
      )
    end
  end
end
