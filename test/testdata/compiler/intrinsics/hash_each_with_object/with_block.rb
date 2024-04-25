# frozen_string_literal: true
# typed: true
# compiled: true

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

h = T.let({a: 1, b: 1, c: 2, d: 3, e: 5, f: 8}, T::Hash[Symbol, Integer])

obj = []
# We can't rely on the ordering of the keys, so break out when we've processed ~half.
i = 0
limit = h.size / 2
result = h.each_with_object(obj) do |(k, v), a|
  break :finished if i == limit
  a << k
  a << v
  i += 1
end

p result
p obj
