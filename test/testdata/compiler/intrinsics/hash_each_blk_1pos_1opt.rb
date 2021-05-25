# frozen_string_literal: true
# typed: true
# compiled: true

# blocks with one required and one optional arg are considered as two args for
# the purposes of Hash#each
result = {a: 1, b: 2, c: :sym}.each do |k, v=:x|
  p " #{k} #{v}"
end

p result
