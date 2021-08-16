# typed: strict

extend T::Sig

sig {returns(Integer)}
def int
  5
end

sig {params(x: T.nilable(Integer)).void}
def f(x)
  if !x.nil?
    x &&= int
    T.reveal_type(x) # error: Revealed type: `Integer`
  end
end
