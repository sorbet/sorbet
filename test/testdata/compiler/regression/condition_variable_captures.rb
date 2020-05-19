# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def self.test(x)
    1.times do
      if x
        puts "yes"
      else
        puts "no"
      end
    end
  end
end

A.test true
A.test false
