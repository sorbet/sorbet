# frozen_string_literal: true
# typed: true
# compiled: true

require_relative './block_return_unsafe__2'

extend T::Sig

sig { returns(Integer) }
def f
  puts (yield_from_2 { return T.unsafe("whoops, this is not an integer") })
  42
end

begin
  puts f
rescue TypeError
  puts "got a TypeError as expected"
end
