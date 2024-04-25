# frozen_string_literal: true
# typed: true
# compiled: true

def foo(**kwargs)
  puts kwargs
end

args = {a: 1, b: 2}
foo(**args)
puts args
