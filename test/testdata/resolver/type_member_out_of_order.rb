# typed: true

Alias1 = T.type_alias {A}
Alias2 = T.type_alias {B}

class A
  extend T::Generic
  X = type_member {{fixed: B}}
end

class B
  extend T::Sig
  extend T::Generic

  X = type_member {{fixed: Integer}}

  sig {returns(X)}
  def test
    10
  end
end

T.reveal_type(B.new.test) # error: Revealed type: `Integer`
