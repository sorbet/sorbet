# typed: true
extend T::Sig

# sealed but not abstract
class ConcreteParent
  extend T::Helpers

  sealed!
end
class Child1 < ConcreteParent; end
class Child2 < ConcreteParent; end
class Child3 < ConcreteParent; end

sig {params(x: ConcreteParent).void}
def foo(x)
  case x
  when Child1
    T.reveal_type(x) # error: Revealed type: `Child1`
  when Child2
  when Child3
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `ConcreteParent` wasn't handled
  end
end

T.reveal_type(ConcreteParent.sealed_subclasses) # error: Revealed type: `T::Set[T.any(T.class_of(Child1), T.class_of(Child2), T.class_of(Child3))]`
