# @typed
require_relative '../../t'

class PreChild < Parent
  Elem = type_member(fixed: String)
end

class Parent
  Elem = type_member

  sig(a: Elem).returns(Elem)
  def foo(a)
    a
  end
end

class Child < Parent
  Elem = type_member(fixed: String)

  def use_foo
    T.assert_type!(foo("foo"), String)
  end
end

module Mixin
  Elem = type_member

  sig(a: Elem).returns(Elem)
  def foo(a)
    a
  end
end

class WithMixin
  include Mixin
  Elem = type_member(fixed: String)
end

class NotATypeVar
  NotAElem = String
end


class ParentWithMultiple
  K = type_member
  V = type_member

  sig(k: K, v: V).returns(K)
  def foo(k, v)
    k
  end
end

class HalfChild < ParentWithMultiple
  K = type_member(fixed: Integer)
  V = type_member
end

class HalfChildOther < ParentWithMultiple
  K = type_member
  V = type_member(fixed: Integer)
end

class FullChild < HalfChild
  K = type_member(fixed: Integer)
  V = type_member(fixed: String)
end

class ParentEnumerable
  include Enumerable
  K = type_member
  V = type_member
  Elem = type_member
end

class ChildEnumerable < ParentEnumerable
  K = type_member(fixed: String)
  V = type_member
  Elem = type_member
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
