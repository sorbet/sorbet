# frozen_string_literal: true
# typed: true
# compiled: true

def test(a, b)
  i = 0
  while i < 1_000_000
    T.must(a)
    T.must(b)
    i += 1
  end
end

test(true, false)
