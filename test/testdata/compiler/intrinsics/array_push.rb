# frozen_string_literal: true
# typed: true
# compiled: true

def test_array_push
  p ([1, 2, 3].push)
  p ([1, 2, 3].push(4))
  p ([1, 2, 3].push(3, 2, 1))
  p ([1, 2, 3].push(1, 2, 3, T.unsafe("greetings")))
  p ([].push(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, T.unsafe("greetings")))

  x = [1, 2, 3]
  x.push(99)

  p x
end

test_array_push

