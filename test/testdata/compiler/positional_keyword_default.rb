# frozen_string_literal: true
# typed: true
# compiled: true

# This illustrates strange behavior in ruby <= 2.6.x where the single hash
# argument gets interpreted as the store for keyword arguments, rather than the
# value of the first positional argument.

class A
  def self.foo(x=nil, y: 0)
    p x
    p y
  end
end

A.foo({})
