# typed: strict
extend T::Sig

module Boolean
  extend T::Helpers
  sealed!
end

class FalseClass
  include Boolean
end

class TrueClass
  include Boolean
end

sig {params(x: Boolean).void}
def foo(x)
  case x
  when TrueClass
    T.reveal_type(x) # error: Revealed type: `TrueClass`
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `FalseClass` wasn't handled
  end
end

T.reveal_type(Boolean.sealed_subclasses) # error: Revealed type: `T::Set[T.any(T.class_of(FalseClass), T.class_of(TrueClass))]`
