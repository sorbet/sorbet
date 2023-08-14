# frozen_string_literal: true

# Ideally we might run tests with warnings enabled, but currently this triggers
# a number of uninitialized instance variable warnings.
# $VERBOSE = true

require 'minitest/autorun'
require 'minitest/spec'
require 'mocha/minitest'

require 'pathname'
require 'tempfile'
require 'json'
require 'subprocess'

require_relative '../lib/sorbet-runtime'

module Critic; end
module Critic::Unit; end
module Minitest; end
class Minitest::Spec; end
class Critic::Unit::UnitTest < Minitest::Spec; end
module Critic::Extensions; end
module Critic::Extensions::TypeExt
  def self.unpatch_types; end
  def self.patch_types; end
end

module Opus; end
module Opus::Types; end
module Opus::Types::Test; end
class Opus::Types::Test::TypesTest < Critic::Unit::UnitTest; end
module Opus::Types::Test::Props; end
module Opus::Types::Test::Props::Private; end
