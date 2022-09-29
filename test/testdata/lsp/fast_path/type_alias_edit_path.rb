# typed: true

class Example
  extend T::Sig

  AliasToInteger =

  sig {params(x: AliasToInteger).void}
  #              ^^^^^^^^^^^^^^ error: `Example::AliasToInteger` is not a class or type alias
  def example(x)
    T.reveal_type(x) # error: `T.untyped`
  end
end
