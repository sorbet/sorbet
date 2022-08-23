# typed: true
extend T::Sig

A = Does::Not::Exist # error: Unable to resolve constant

sig {returns(A)} # error: Constant `A` is not a class or type alias
def example
end
