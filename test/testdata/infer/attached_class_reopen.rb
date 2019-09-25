# typed: true

class A
  extend T::Sig
  extend T::Generic

  X = type_member(fixed: Integer)

  sig {returns(X)}
  def test_X
    10
  end
end

class B; end

class A
  Y = type_member(fixed: String)

  sig {returns(Y)}
  def test_Y
    "foo"
  end
end

T.reveal_type(A.new) # error: Revealed type: `A`
T.reveal_type(A.new.test_X) # error: Revealed type: `Integer`
T.reveal_type(A.new.test_Y) # error: Revealed type: `String`
