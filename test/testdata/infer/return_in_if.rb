# typed: true

extend T::Sig

sig {params(cond: T::Boolean).returns(Integer)}
def foo(cond)
  if (cond)
    a = 1
  else
    return 1
    a = "1"
    #   ^^^ error: This expression appears after an unconditional return
  end
  a
end
