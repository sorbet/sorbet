# frozen_string_literal: true
# typed: true
# compiled: true

require_relative('cross_file_compiled_final_method2__3')

module Utils
  extend T::Sig
  extend T::Helpers

  def self.call_a_final_method
    OtherUtils.final_method
  end
end
