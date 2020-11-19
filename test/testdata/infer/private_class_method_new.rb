# typed: strict

class A
  private_class_method :new

  x = new
  T.reveal_type(x) # error: Revealed type: `T.attached_class (of A)`
end

y = A.new # error: Non-private call to private method `new`
T.reveal_type(y) # error: Revealed type: `A`

class B
  extend T::Sig
  private_class_method :new

  sig {params(arg0: Integer).void}
  def initialize(arg0); end

  new # error: Not enough arguments provided for method `B#initialize`
  new(0)
end

y = B.new(0) # error: Non-private call to private method `new`
T.reveal_type(y) # error: Revealed type: `B`
