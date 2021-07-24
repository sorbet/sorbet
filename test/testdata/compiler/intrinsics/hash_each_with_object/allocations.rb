# frozen_string_literal: true
# typed: true
# compiled: true

h = {a: 1, b: 1, c: 2, d: 3, e: 5, f: 8}

result = h.each_with_object([]) do |kv, a|
  a << kv
end

p result

# Ruby allocates a fresh array each iteration and so should we.
objids = result.map(&:object_id)
uniq_ids = objids.uniq

p objids.size == uniq_ids.size
