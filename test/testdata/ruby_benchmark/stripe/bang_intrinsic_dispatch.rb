# frozen_string_literal: true
# typed: true
# compiled: true

i = 0
while i < 10_000_000
  !"foo"
  i += 1
end
