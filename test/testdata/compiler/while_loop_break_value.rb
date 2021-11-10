# typed: true
# frozen_string_literal: true
# compiled: true

# The VM doesn't allow break to pass out a value when used in a while loop, and
# the compiler should not allow that either.

x = while true
      break 10
    end

puts "x = #{x}"

