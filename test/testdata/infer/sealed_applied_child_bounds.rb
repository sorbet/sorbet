# typed: strict

class Parent
  extend T::Helpers
  extend T::Generic
  sealed!
  abstract!
  A = type_member
end

class Child1 < Parent
  A = type_member {{lower: Integer}}
end

class Child2 < Parent
  A = type_member {{upper: Integer}}
end

extend T::Sig

sig {params(x: Parent[String]).void}
def foo(x)
  case x
  when Child1
    T.reveal_type(x) # error: Revealed type: `Child1[T.untyped]`
  when Child2
    T.reveal_type(x) # error: Revealed type: `Child2[T.untyped]`
  else T.absurd(x)
  end
end
