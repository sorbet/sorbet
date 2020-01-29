# frozen_string_literal: true
# typed: true
# compiled: true

class Main
  extend T::Sig
  sig {void}
  def self.main
    puts 243
  end
end

Main.main
