# frozen_string_literal: true
# typed: true
# compiled: true

def test_array_diff
  p ([] - [1,2,3])
  p ([1,2,3] - [])
  p ([1,2,3] - [1,2])
  p ([1,2,3] - [2,2,3])
  p ([1,2,3] - [1,2,3,4])
  p ([1,"3",2,0] - [1,2,"3"])
end

test_array_diff

