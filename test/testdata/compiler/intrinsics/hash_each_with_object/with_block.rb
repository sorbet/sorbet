# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

h = T.let({a: 1, b: 1, c: 2, d: 3, e: 5, f: 8}, T::Hash[Symbol, Integer])

result = h.each_with_object([]) do |kv, a|
  a << kv
end

p result

h = T.let({a: 1, b: 1, c: 2, d: 3, e: 5, f: 8}, T::Hash[Symbol, Integer])

result = h.each_with_object([]) do |(k, v), a|
  a << k
  a << v
end

p result

# INITIAL-COUNT-2: call i64 @sorbet_callIntrinsicInlineBlock
