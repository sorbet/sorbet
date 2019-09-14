# typed: true

module M
  def foo_from_m
  end

  def method_in_parent
  end
end

class A
  include M
  def foo_from_a
  end

  def bar_from_a
  end

  def method_in_child_but_long
  end
end

A.new.foo # error: does not exist
#        ^ completion: foo_from_a, foo_from_m

A.new.method_in_ # error: does not exist
#            ^ completion: method_in_child_but_long, method_in_parent
