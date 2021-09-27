# frozen_string_literal: true
# typed: true
# compiled: true

module Helpers
  extend T::Sig

  sig {returns(Float).checked(:never)}
  module_function def sample_rate
    0.01
  end
end

p Helpers.sample_rate
