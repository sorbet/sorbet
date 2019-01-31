# frozen_string_literal: true

require 'minitest/autorun'
require 'minitest/spec'
require 'mocha/minitest'

require 'pathname'

require_relative '../lib/sorbet-runtime'

module Boolean; end
class TrueClass
  include Boolean
end
class FalseClass
  include Boolean
end

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

module Chalk; end
module Chalk::Log; end
module Chalk::Log::CLevels; end
class Chalk::Log::CLevels::Sheddable; end

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
Chalk::Tools::RedactionUtils::RedactionDirectiveSpec = T.type_alias(T.any(
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
))

module Opus; end
module Opus::Types; end
module Opus::Types::Test; end
class Opus::Types::Test::TypesTest < Critic::Unit::UnitTest; end
module Opus::Types::Test::Props; end
module Opus::Sensitivity; end
module Opus::Sensitivity::PIIable; end
module Opus::Sensitivity::Utils
  def self.normalize_sensitivity_and_pii_annotation(value)
    value
  end
end
class Opus::Enum; end
module Opus::Breakage; end
module Opus::CI; end
module Opus::Projects
  def self.storage
  end
end

module Opus::Error
  def self.hard(message, *)
    raise message
  end

  def self.soft(*); end
end

module Opus::Log
  def self.info(*); end
end

module Opus::Project
  def self.fetch(*); end
end

module Opus::Sys
  def self.testing?
    true
  end
end
