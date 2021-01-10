# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::TopLevel < Critic::Unit::UnitTest
  TOP_LEVEL_FIXTURE = "#{__dir__}/fixtures/top_level.rb"

  it 'works with top level sigs' do
    result = Subprocess.check_output(["ruby", TOP_LEVEL_FIXTURE])
    assert_equal(<<~EXPECTED, result)
      called main#foo
      Expected type Integer, got type String with value "nope"
      called main.bar
      Expected type Symbol, got type Float with value 0.0
    EXPECTED
  end
end
