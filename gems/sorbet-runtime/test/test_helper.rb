# frozen_string_literal: true

# Ideally we might run tests with warnings enabled, but currently this triggers
# a number of uninitialized instance variable warnings.
# $VERBOSE = true

require 'minitest/autorun'
require 'minitest/spec'
require 'mocha/minitest'

require 'pathname'
require 'tempfile'

require_relative '../lib/sorbet-runtime'

module Critic; end
module Critic::Unit; end
module MiniTest; end
class MiniTest::Spec; end
class Critic::Unit::UnitTest < MiniTest::Spec; end
module Critic::Extensions; end
module Critic::Extensions::TypeExt
  def self.unpatch_types
  end
  def self.patch_types
  end
end

module Opus; end

module Chalk; end
module Chalk::Tools; end
module Chalk::Tools::RedactionUtils
  def self.redact_with_directive(value, opts = [])
    opts = [opts] unless opts.is_a?(Array)
    case opts[0]
    when :redact_digits
      value.gsub(/\d/, '*')
    when :truncate
      T::Utils.string_truncate_middle(value, opts[1], 0)
    else
      value
    end
  end
end
Chalk::Tools::RedactionUtils::RedactionDirectiveSpec = T.type_alias do
  T.any(
    T.enum([
      :redact_digits,
      :redact_digits_except_last4,
      :redact_card,
      :redact_all,
      :truncate,
    ]),
    [T.enum([:truncate]), Integer],
    [T.enum([:truncate_middle]), Integer, Integer],
    [T.enum([:redact_middle]), Integer, Integer],
    [T.enum([:replace]), String],
  )
end

module Opus::Types; end
module Opus::Types::Test; end
class Opus::Types::Test::TypesTest < Critic::Unit::UnitTest; end
module Opus::Types::Test::Props; end
