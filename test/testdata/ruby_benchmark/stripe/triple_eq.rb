# frozen_string_literal: true
# typed: true
# compiled: true

x = T.unsafe(nil)

i = 0
while i < 10_000_000

  NilClass.===(x)

  i += 1
end

puts i
