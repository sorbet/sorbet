# typed: strict
extend T::Sig

sig {returns(Integer)}
def int
  5
end

sig {returns(Integer)}
def newlocal
  x ||= int
  T.reveal_type(x) # error: Revealed type: `Integer`
  x
end

