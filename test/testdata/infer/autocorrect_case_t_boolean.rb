# typed: true
extend T::Sig

sig {params(input: T.untyped).returns(T::Boolean)}
def get_value(input)
  case input
  when T::Boolean
    #  ^^^^^^^^^^ error: Call to method `===` on `T::Boolean` mistakes a type for a value
    true
  else
    false
  end
end

sig {params(input: T.untyped).returns(T::Boolean)}
def test(input)
  if input === T::Boolean
    true
  else
    false
  end
end

MyTrue = T.type_alias { TrueClass }

x = 0
case x
when T.untyped
  #  ^^^^^^^^^ error: Call to method `===` on `T.untyped` mistakes a type for a value
  T.reveal_type(x)
# ^^^^^^^^^^^^^^^^ error: Revealed type: `Integer(0)`
when T.noreturn
  #  ^^^^^^^^^^ error: Call to method `===` on `T.noreturn` mistakes a type for a value
  T.reveal_type(x)
# ^^^^^^^^^^^^^^^^ error: Revealed type: `Integer(0)`
when MyTrue
  #  ^^^^^^ error: Call to method `===` on `TrueClass` mistakes a type for a value
  T.reveal_type(x)
# ^^^^^^^^^^^^^^^^ error: Revealed type: `Integer(0)`
end
