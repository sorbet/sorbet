# typed: true

extend T::Sig

class Box
  extend T::Sig
  extend T::Generic

  Elem = type_member

  DefaultValue = 0

  sig {params(x: Elem).void}
  def example(x)
    T.reveal_type(x) # error: `Box::Elem`
    T.reveal_type(DefaultValue) # error: `Integer`
  end
end

sig {params(x: Box[Integer]).void}
def main(x)
  T.reveal_type(x) # error: `Box[Integer]`
end
