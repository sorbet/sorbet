# typed: true
# spacer for exclude-from-file-update

class Box
  extend T::Sig
  extend T::Generic
  Elem = type_member

  sig { params(x: MyInteger).void }
  def example(x)
    T.reveal_type(x) # error: `Integer`
  end
end
