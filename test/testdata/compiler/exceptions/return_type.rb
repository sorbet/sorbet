# frozen_string_literal: true
# typed: true
# compiled: true

module Main
  extend T::Sig

  sig {returns(Integer)}
  def self.foo
    begin
      raise "test"
    rescue
      return T.unsafe("hi")
    end
  end
end

begin
  puts Main.foo
rescue TypeError
  puts "Got TypeError as expected"
end
