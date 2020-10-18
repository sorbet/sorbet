# typed: strict

class Parent
  extend T::Helpers
  extend T::Generic
  sealed!
  abstract!
  A = type_member
end

class Child1 < Parent
  A = type_member(lower: Integer)
end

class Child2 < Parent
  A = type_member
end

extend T::Sig

sig {params(x: Parent[String]).void}
def foo(x)
  case x
  when Integer
  when Child1
    # This is wrong: the revealed type should not be allowed to exists, because
    # `String` is not a super type of `Integer`, which is a lower bound of `Child1::A`
    T.reveal_type(x) # error: Revealed type: `Child1[String]`
  when Child2
    T.reveal_type(x)
  else T.absurd(x)
  end
end
