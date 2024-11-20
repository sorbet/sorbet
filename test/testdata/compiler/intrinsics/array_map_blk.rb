# frozen_string_literal: true
# typed: true
# compiled: true

# Break is only used in the second call to `Array#map`, so we should see that the first version uses the `noBreak`
# variant of `sorbet_callIntrinsicInlineBlock`, and the second does not.



# Additionally, in the optimized output, we should see exactly one call to `sorbet_rb_iterate` (from the use of
# `sorbet_callIntrinsicInlineBlock`) in the Init function, as the root static init will be inlined there.


result = [1, 2, 3].map do |x|
  puts "-- #{x} --"
  x
end

p result

result = [1, 2, 3, 4, 5, 6, 7, 8].map do |x|
  puts "-- #{x} --"
  break :finished if x == 6
end

p result

