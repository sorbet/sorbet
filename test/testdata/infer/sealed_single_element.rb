# typed: strict

extend T::Sig

module IFoo
  extend T::Helpers
  sealed!

  class A < T::Struct
    include IFoo
  end
end

x = T.let(IFoo::A.new, IFoo)
T.reveal_type(IFoo::A === x) # error: Revealed type: `TrueClass`

sig {params(foo: IFoo).returns(Integer)}
def foo!(foo)
  # This case is exhaustive
  res = case foo
  when IFoo::A then 0
  end

  res
end


class Parent
  extend T::Helpers

  sealed!
end

class Child < Parent; end

sig {params(x: Parent).returns(Integer)}
def test1(x)
  # This case is not exhaustive (Parent.new is a possible value)
  res = case x
  when Child then 0
  end

  res # error: Expected `Integer` but found `T.nilable(Integer)`
end

bad = T.let(Child.new, Parent)
T.reveal_type(Child === bad) # error: Revealed type: `T::Boolean`

class AbsParent
  extend T::Helpers

  abstract!
  sealed!
end

class ChildOfAbs < AbsParent; end

sig {params(x: AbsParent).returns(Integer)}
def test2(x)
  # This case is exhaustive
  res = case x
  when ChildOfAbs then 0
  end

  res
end

abs = T.let(ChildOfAbs.new, AbsParent)
T.reveal_type(ChildOfAbs === abs) # error: Revealed type: `TrueClass`
