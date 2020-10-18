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
    # TODO(jez)
    # This is wrong: the revealed type should not be allowed to exists, because
    # `String` is not a super type of `Integer`, which is a lower bound of `Child1::A`
    T.reveal_type(x) # error: Revealed type: `Child1[String]`
  when Child2
    # TODO(jez) This is also wrong, for the same reason.
    T.reveal_type(x) # error: Revealed type: `Child2[String]`
  else T.absurd(x)
  end
end
