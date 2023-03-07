# typed: strict
extend T::Sig

sig {params(x: String).returns(Integer)}
def foo(x)
  if !x == "foo"
    0 # error: This code is unreachable
  else
    1
  end
end
