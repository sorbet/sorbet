# frozen_string_literal: true
# typed: true
# compiled: true

# Many of our benchmarks use a while loop with `+=` over 1M items.
#
# Use this benchmark as a baseline to check how much time is spent in an
# operation vs spent in the while loop & loop counter.

i = 0
while i < 1_000_000

  # ... potential extra stuff ...

  i += 1
end

puts i
