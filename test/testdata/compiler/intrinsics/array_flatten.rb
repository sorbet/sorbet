# frozen_string_literal: true
# typed: true
# compiled: true

def test_array_flatten
  p ([].flatten)
  p ([1, 2, 3].flatten)
  p ([1, [2, 3]].flatten)
  p ([1, [2, [3, 4]]].flatten)
  p ([1, [2, [3, 4]]].flatten(0))
  p ([1, [2, [3, 4]]].flatten(1))
  p ([1, [2, [3, 4]]].flatten(2))
  p ([1, [2, [3, 4]]].flatten(3))
  p ([1, [2, [3, 4]]].flatten(4))
end

test_array_flatten

