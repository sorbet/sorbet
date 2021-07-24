# frozen_string_literal: true
# typed: true
# compiled: true

result = {a: 1, b: 2, c: :sym}.each do |k: "not provided", v: "also not"|
  p " #{k} #{v}"
end

p result


