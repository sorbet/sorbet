# frozen_string_literal: true
# typed: true

module Utils
  extend T::Sig
  extend T::Helpers

  final!

  sig(:final) {void}
  def self.final_method
    puts "final method!"
  end
end

require_relative 'cross_file_interpreted_final_method__2'
