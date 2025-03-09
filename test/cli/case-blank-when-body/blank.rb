# typed: true

extend T::Sig

sig { params(x: Integer).returns(String) }
def foo(x)
  case x
  when 1
  when 2
    "a"
  else
    "b"
  end
end
