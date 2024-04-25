# frozen_string_literal: true
# typed: true
# compiled: true

# Lambda is defined at top of file.
x0 = 3

F0 = -> (y) { return x0 + y }

# Lambda is defined in a module.
module M
  x1 = 4

  F1 = -> (y) { return x1 + y }
end

class C
  # Lambda is defined in a class.
  x2 = 5

  F2 = -> (y) { return x2 + y }

  def self.f3
    # Lambda is defined in a class method.
    x3 = 6

    -> (y) { return x3 + y }
  end

  def f4
    # Lambda is defined in an instance method.
    x4 = 7

    -> (y) { return x4 + y }
  end
end

def justyield(x)
  yield x
end

F5 = -> () do
  # Lambda is defined inside a block.
  justyield(8) do |x5|
    -> (y) { return x5 + y }
  end
end

# Two-arg lambda.
x8 = 11

F8 = -> (y,z) { return x8 + y + z }
