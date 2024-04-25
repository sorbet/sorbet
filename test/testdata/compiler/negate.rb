# frozen_string_literal: true
# typed: true
# compiled: true

class Amount
  extend T::Sig

  attr_reader :amount

  def initialize(x)
    @amount = T.let(x, Integer)
  end

  sig {params(other: Amount).returns(Amount)}
  def +(other)
    Amount.new(self.amount + other.amount)
  end

  sig {params(other: Amount).returns(Amount)}
  def -(other)
    self + (-other)
  end

  sig {returns(Amount)}
  def -@
    Amount.new(-self.amount)
  end
end

five = Amount.new(5)
four = Amount.new(4)

p (five + four).amount
p (five - four).amount
p (four - five).amount
