# typed: true

class Example
  extend T::Sig
  extend T::Generic

  X = type_member

  sig {params(x: X).void}
  def example(x)
    T.reveal_type(x) # error: `Example::X`
  end
end
