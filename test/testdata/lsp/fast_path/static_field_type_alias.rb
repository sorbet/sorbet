# typed: true

class AliasContainer
  extend T::Sig

  ContainedThing = T.type_alias {T.any(Float, Symbol)}

  sig {params(x: ContainedThing).void}
  def example(x)
    T.reveal_type(x) # error: `T.any(Float, Symbol)`
  end
end
