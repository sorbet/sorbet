# typed: true

class AliasContainer
  extend T::Sig

  ContainedThing = T.type_alias {T.any(Float, Symbol)}

  sig {params(x: ContainedThing).returns(ContainedThing)}
  def example(x=0.0)
    T.reveal_type(x) # error: `T.any(Float, Symbol)`
  end
end

class AliasContainerChild < AliasContainer
end
