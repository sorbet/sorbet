# frozen_string_literal: true
# typed: strict

module Opus; end
class Opus::Log

  module CLevels
    Sheddable = Chalk::Log::CLevels::Sheddable
    SheddablePlus = Chalk::Log::CLevels::SheddablePlus
    Critical = Chalk::Log::CLevels::Critical
    CriticalPlus = Chalk::Log::CLevels::CriticalPlus
  end
end
