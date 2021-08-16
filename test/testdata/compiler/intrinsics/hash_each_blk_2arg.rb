# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

# "Normal" block
result = {a: 1, b: 2, c: :sym}.each do |k, v|
  p " #{k} => #{v}"
end

p result

h = {a: 1, b: 2, c: :sym, d: :else, e: :other, f: 10}
i = 0
limit = h.size / 2
result = h.each do |k, v|
  break :finished if i == limit
  p " #{k} => #{v}"
  i += 1
end

p result

# INITIAL-COUNT-2: call i64 @sorbet_callIntrinsicInlineBlock
# INITIAL-NOT: call i64 @sorbet_callIntrinsicInlineBlock
