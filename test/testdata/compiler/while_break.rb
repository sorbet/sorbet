# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def initialize
    x = 0
    while true
      puts x
      if x >= 3
        break
      end
      x += 1
    end

    puts x
  end
end

A.new
