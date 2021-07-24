# frozen_string_literal: true
# typed: true
# compiled: true

i = 0
while i < 100_000
  !"foo"
  i += 1
end
