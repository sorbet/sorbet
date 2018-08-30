# typed: true

extend T::Helpers

sig(cond: T.any(TrueClass, FalseClass)).returns(Integer)
def foo(cond)
  if (cond)
    a = 1
  else
    return 1
    a = "1"
  end
  a
end
