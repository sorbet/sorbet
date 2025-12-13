# frozen_string_literal: true
# typed: true
# compiled: true

def foo(**kwargs)
end

i = 0
while i < 1_000_000

  args = {a: 1, b: 2}
  foo(**args)

  i += 1
end

puts i
