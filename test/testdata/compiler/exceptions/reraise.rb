# frozen_string_literal: true
# typed: true
# compiled: true

# Tests that exceptions that aren't handled are re-raised after the ensure block
# runs.

class E < StandardError; end

class A
  def self.test
    begin
      raise E, "foo"
    ensure
      puts "ensure"
    end
  end
end

begin
  A.test
rescue E => e
  puts e
end
