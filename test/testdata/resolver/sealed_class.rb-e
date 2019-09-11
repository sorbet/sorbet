# typed: true
extend T::Sig

class AbstractParent
  extend T::Helpers

  sealed!
  abstract!
end
class Child1 < AbstractParent; end
class Child2 < AbstractParent; end
class Child3 < AbstractParent; end

sig {params(x: AbstractParent).void}
def foo(x)
  case x
  when Child1
    T.reveal_type(x) # error: Revealed type: `Child1`
  when Child2
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `Child3` wasn't handled
  end
end
