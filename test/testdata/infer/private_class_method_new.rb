# typed: strict

class A
  private_class_method :new

  x = new
  T.reveal_type(x) # error: Revealed type: `T.attached_class (of A)`
end

y = A.new # error: Non-private call to private method `A.new`
T.reveal_type(y) # error: Revealed type: `A`
