# typed: true
extend T::Sig

A = Does::Not::Exist # error: Unable to resolve constant

sig {returns(A)}
def example
end
