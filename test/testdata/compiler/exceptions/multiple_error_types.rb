# frozen_string_literal: true
# typed: true
# compiled: true

# Tests that it's possible to rescue from multiple different error types

class E1 < StandardError; end
class E2 < StandardError; end

class A
  def self.test(x)
    begin
      if x
        raise E1, "first"
      else
        raise E2, "second"
      end

    rescue E1 => e
      puts "Caught E1: #{e.message}"

    rescue E2 => e
      puts "Caught E2: #{e.message}"

    end
  end
end

A.test true
A.test false
