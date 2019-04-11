# typed: true

extend T::Sig

sig {params(cond: T::Boolean).returns(Integer)}
def foo(cond)
  if (cond)
    a = 1
  else
    return 1
    a = "1"
  end
  a
end
