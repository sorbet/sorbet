# typed: true

Const = Struct.new(:something, keyword_init: true) do
  extend T::Sig

  sig { params(something: Integer).void }
  def initialize(something: 0)
    T.reveal_type(something) # error: Integer
  end
end
