# frozen_string_literal: true
# typed: true
# compiled: true

class Main
  extend T::Sig
  sig {void.checked(:never)}
  def self.main
    nil
  end
end

i = 0
while i < 10_000_000

  Main.main

  i += 1
end

puts i
