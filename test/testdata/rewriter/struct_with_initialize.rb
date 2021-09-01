# typed: true

Const = Struct.new(:something, keyword_init: true) do
  extend T::Sig

  sig { params(something: Integer).void }
  def initialize(something: 0)
    T.reveal_type(something) # error: Integer
  end
end

AnotherConst = Struct.new(:something, keyword_init: true) do
  extend T::Sig

  sig { params(something: Integer).void }
  def initialize(something: 0)
    T.reveal_type(something) # error: Integer
  end

  def other_method
    "more statements".upcase
  end
end

ConstWithSomethingBeforeInit = Struct.new(:something, keyword_init: true) do
  extend T::Sig

  sig { returns(String) }
  def other_method
    "more statements".upcase
  end

  sig { params(something: Integer).void }
  def initialize(something: 0)
    T.reveal_type(something) # error: Integer
  end
end
