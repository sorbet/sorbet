# frozen_string_literal: true
# typed: true
# compiled: true

class Bad
  def !
    puts "bad bang overload"
    true
  end
end

module Main
  def self.test
    puts !true
    puts !false
    puts !nil
    puts !"hello"
    puts !Bad.new
  end
end

Main.test
