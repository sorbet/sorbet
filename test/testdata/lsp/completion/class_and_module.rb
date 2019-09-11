# typed: true

module M
  def foo_from_m
  end
end

class A
  include M
  def foo_from_a
  end

  def bar_from_a
  end

  def car
  end
end

A.new.foo # error: does not exist
#        ^ completion: foo_from_a, foo_from_
