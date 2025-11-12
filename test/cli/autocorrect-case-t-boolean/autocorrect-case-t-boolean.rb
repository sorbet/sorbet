# typed: true
extend T::Sig

sig {params(input: T.untyped).returns(T::Boolean)}
def get_value(input)
  case input
  when T::Boolean
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
