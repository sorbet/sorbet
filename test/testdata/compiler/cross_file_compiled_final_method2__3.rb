# frozen_string_literal: true
# typed: true
# compiled: true

module OtherUtils
  extend T::Sig
  extend T::Helpers

  sig(:final) {void}
  def self.final_method
    p "a final method!"
  end
end
