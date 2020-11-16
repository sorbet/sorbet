# typed: strict

class A
  private_class_method :new

  x = new
  T.reveal_type(x) # error: Revealed type: `T.attached_class (of A)`
end

# TODO(jez) This is blocked by this bug: https://github.com/sorbet/sorbet/issues/3662
y = A.new # error: Non-private call to private method `A.new`
T.reveal_type(y) # error: Revealed type: `A`
