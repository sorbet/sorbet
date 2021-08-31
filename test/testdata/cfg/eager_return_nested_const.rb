# typed: true
extend T::Sig

class Wrapper
  X = T.let('this is a string', String)
end

sig {params(x: String).returns(Integer)}
def one_branch_bad_return(x)
  if Random.rand(2).even?
    Wrapper::X # error: Expected `Integer` but found `String` for method result type
  else
    T.unsafe(nil)
  end
end
