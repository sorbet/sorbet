# frozen_string_literal: true
# typed: true
# compiled: true

def test_array_first
  p [].first
  p [1, 2, 3].first
  p [1, 2, 3].first(0)
  p [1, 2, 3].first(1)
  p [1, 2, 3].first(2)
  p [1, 2, 3].first(3)
  p [1, 2, 3].first(4)
end

test_array_first

