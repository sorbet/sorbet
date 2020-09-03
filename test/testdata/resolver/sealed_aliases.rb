# typed: true
extend T::Sig

class Parent;
  extend T::Helpers
  abstract!
  sealed!
end
P = Parent

class Child1 < P; end
C1 = Child1
class Child2 < P; end
C2 = Child2

sig {params(x: P).void}
def foo(x)
  case x
  when C1
    T.reveal_type(x) # error: Revealed type: `Child1`
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `Child2` wasn't handled
  end
end

T.reveal_type(Parent.sealed_subclasses) # error: Revealed type: `T::Set[T.any(T.class_of(Child1), T.class_of(Child2))]`
