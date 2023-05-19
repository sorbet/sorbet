# typed: true
extend T::Sig

module Parent
  extend T::Helpers

  sealed!
end
class Child1; include Parent; end
class Child2; include Parent; end
class Child3; include Parent; end

sig {params(x: Parent).void}
def foo(x)
  case x
  when Child1
    T.reveal_type(x) # error: Revealed type: `Child1`
  when Child2
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `Child3` wasn't handled
  end
end

T.reveal_type(Parent.sealed_subclasses) # error: Revealed type: `T::Set[T.any(T.class_of(Child1), T.class_of(Child2), T.class_of(Child3))]`
