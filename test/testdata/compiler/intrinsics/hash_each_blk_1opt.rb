# frozen_string_literal: true
# typed: true
# compiled: true

# blocks with one optional arg are considered as single-arg blocks for the
# purposes of Hash#each.
result = {a: 2, b: 3, c: :arg}.each do |a=:x|
  p " #{a}"
end

p result
