# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

class A
  extend T::Sig

  attr_reader :proc

  def initialize(p)
    @proc = p
  end
end

sig {returns A}
def a
  A.new(lambda {return T.unsafe(5)})
end

sig {params(a: A).returns(Integer)}
def b(a)
  a.proc.call
end

p b(a())
