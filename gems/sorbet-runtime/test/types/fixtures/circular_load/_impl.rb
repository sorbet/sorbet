# typed: true
require_relative '../../../test_helper'

module Opus::Types::Test
  module Fixtures
    module CircularLoad
      autoload :Parent,   "#{__dir__}/parent.rb"
      autoload :OneOrTwo, "#{__dir__}/one_or_two.rb"
      autoload :Child1,   "#{__dir__}/child1.rb"
      autoload :Child2,   "#{__dir__}/child2.rb"
    end
  end
end
