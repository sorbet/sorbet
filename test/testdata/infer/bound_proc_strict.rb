# typed: strict

class UntypedBind
  extend T::Sig

  sig { void }
  def foo
    T.reveal_type(self) # error: type: `T.self_type (of UntypedBind)`
    T.bind(self, T.untyped)
    T.reveal_type(self) # error: type: `T.untyped`
    T.bind(self, UntypedBind)
    T.reveal_type(self) # error: type: `UntypedBind`
  end
end
