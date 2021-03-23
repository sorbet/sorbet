# frozen_string_literal: true
# typed: true
# compiled: true

def print_method
  p "wrong method"
end

class Main
  def initialize
    @x = 5
  end

  def print_method
    p "right method"
  end
end

Main.define_method("show") do
  p @x
  print_method
end

T.unsafe(Main.new).show

