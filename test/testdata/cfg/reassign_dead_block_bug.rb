# frozen_string_literal: true
# typed: true

2.times do
  x = 1
  return 1
  x = 10
  #   ^^ error: This expression appears after an unconditional return
end
