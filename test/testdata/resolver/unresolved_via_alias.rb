# typed: true
extend T::Sig

class A
end
AliasToA = A

sig {returns(AliasToA::B)}
def example; end
