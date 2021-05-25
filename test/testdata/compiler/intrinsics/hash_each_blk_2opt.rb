# frozen_string_literal: true
# typed: true
# compiled: true

# blocks with two optional args are considered as two-arg blocks for the
# purposes of Hash#each.
result = {a: 2, b: 3, c: :arg}.each do |x=:x, y=:y|
  p " #{x} #{y}"
end

p result
