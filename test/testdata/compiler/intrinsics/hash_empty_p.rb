# frozen_string_literal: true
# typed: true
# compiled: true

def test_empty_p
  h = {}
  p (h.empty?)

  h = {x: 33}
  p (h.empty?)
end

