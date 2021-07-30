# frozen_string_literal: true
# typed: true
# compiled: true
module GCD
  def self.gcd(a, b)
    while (a != b)
      if a > b
        a -= b
      else
        b -= a
      end
    end
    a
  end
end
