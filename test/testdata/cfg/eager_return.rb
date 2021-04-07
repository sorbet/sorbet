# typed: true
extend T::Sig

sig {params(x: String).returns(Integer)}
def one_branch_bad_return(x)
  if Random.rand(2).even?
    x
  else
    T.unsafe(nil)
  end
end
