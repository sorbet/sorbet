# typed: true
extend T::Sig

sig {returns(Integer)}
def no_dead_double_return
  if Random.rand(2).even?
    return 0
  else
    return T.unsafe(nil)
  end
end
