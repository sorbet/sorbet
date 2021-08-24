# typed: true
# compiled: true
# frozen_string_literal: true

# This test exercises a few surprising interactions between return in an
# `ensure` block and other non-local control flow constructs.

def takes_block
  begin
    yield
  ensure
    return 10
  end
end

# Exercise the case where a function that returns from ensure will override the
# value returned with `break` from a block
puts takes_block {|| break "hello!" }

def returns_from_block
  # This won't actually return "hello!", as the return value is overwritten by
  # the `return` present in the ensure block
  puts takes_block {|| return "hello!" }

  # As the return exception was handled by the ensure, this statement will run.
  puts "surprisingly, this print statement runs!"
end

puts returns_from_block

def problems?
  begin
    # This exception should be eaten by the use of `return` in the `ensure`
    raise "problems"
  ensure
    return "no problems"
  end
end

puts problems?

puts "end"
