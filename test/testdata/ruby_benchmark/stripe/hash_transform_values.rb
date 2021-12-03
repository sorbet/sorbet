# frozen_string_literal: true
# typed: true
# compiled: true

i = 0

# 8 is a tipping point on the performance of this benchmark. Initially the compiler is about 2x faster than the
# interpreter, but anything above 8 starts to stretch out the interpreted time substantially, and really show the
# benefit of having compiled blocks.
xs = T.let((:a..).take(8).zip(1..).to_h.compact, T::Hash[Symbol, Integer])

while i < 1_000_000
  xs.transform_values{|a| a+1}.length

  i += 1
end

puts i
