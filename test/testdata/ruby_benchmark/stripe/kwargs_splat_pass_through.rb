# frozen_string_literal: true
# typed: true
# compiled: true

def bar(**kwargs)
end

def foo(**args)
  bar(**args)
end

args = {a: 1, b: 2}
i = 0
while i < 1_000_000
  foo(**args)
  i += 1
end

puts i
