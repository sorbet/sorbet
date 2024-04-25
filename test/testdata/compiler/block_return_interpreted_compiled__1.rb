# frozen_string_literal: true
# typed: true
# compiled: false

# This defines "yield_from_2", which simply yields with no block arg, and "yield_with_arg_from_2" which yields with
# the argument passed to the block:
require_relative './block_return_interpreted_compiled__2'

def f
  puts (yield_from_2 { return 42 })
  raise "Expected to return from f"
end

puts (1000000 + f + 1000)

class C
  def self.f
    puts (yield_from_2 { return 45 })
    raise "expected to return from C.f"
  end

  def g
    puts (yield_from_2 { return 46 })
    raise "expected to return from C#g"
  end
end

puts (2000000 + C.f + 2000)
puts (3000000 + C.new.g + 3000)

def k
  puts (yield_with_arg_from_2(26) { |x| return (x*2) })
  raise "Expected to return from k"
end

puts (4000000 + k + 4000)

# No output is expected from the following, and we should return from static init.
puts (yield_from_2 { return 53 })
raise "Expected to return from file root"
