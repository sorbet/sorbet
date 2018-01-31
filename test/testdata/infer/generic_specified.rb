# @typed
require_relative '../../t'

class PreChild < Parent
  Elem = T.type(fixed: String)
end

class Parent
  Elem = T.type

  sig(a: Elem).returns(Elem)
  def foo(a)
    a
  end
end

class Child < Parent
  Elem = T.type(fixed: String)
end

module Mixin
  Elem = T.type

  sig(a: Elem).returns(Elem)
  def foo(a)
    a
  end
end

class WithMixin
  include Mixin
  Elem = T.type(fixed: String)
end

class NotATypeVar
  NotAElem = String
end


class ParentWithMultiple
  K = T.type
  V = T.type

  sig(k: K, v: V).returns(K)
  def foo(k, v)
    k
  end
end

class HalfChild < ParentWithMultiple
  K = T.type(fixed: Integer)
  V = T.type
end

class HalfChildOther < ParentWithMultiple
  K = T.type
  V = T.type(fixed: Integer)
end

class FullChild < HalfChild
  K = T.type(fixed: Integer)
  V = T.type(fixed: String)
end

class ParentEnumerable
  include Enumerable
  K = T.type
  V = T.type
  Elem = T.type
end

class ChildEnumerable < ParentEnumerable
  K = T.type(fixed: String)
  V = T.type
  Elem = T.type
end

def main
  a = Child.new.foo('a')
  T.assert_type!(a, String)

  a = PreChild.new.foo('a')
  T.assert_type!(a, String)

  a = WithMixin.new.foo('a')
  T.assert_type!(a, String)

  a = ParentWithMultiple[Symbol, String].new.foo(:a, 'b')
  T.assert_type!(a, Symbol)

  a = HalfChild[String].new.foo(1, 'b')
  T.assert_type!(a, Integer)

  a = HalfChildOther[String].new.foo('a', 1)
  T.assert_type!(a, String)

  a = FullChild.new.foo(1, 'b')
  T.assert_type!(a, Integer)

  a = ChildEnumerable[Integer, String].new.min
  T.assert_type!(a, String)
end
main
