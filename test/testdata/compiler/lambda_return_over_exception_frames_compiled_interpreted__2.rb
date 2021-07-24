# frozen_string_literal: true
# typed: true
# compiled: false

F6 = -> () do
  # Lambda is defined inside rescue.
  x6 = 100000

  begin
    raise "foo"
  rescue
    -> (y) { return x6 + y }
  ensure
    x6 = 9
  end
end

F7 = -> () do
  begin
    # Lambda is defined inside begin.
    x7 = 10

    -> (y) { return x7 + y }
  rescue
    "oops"
  end
end
