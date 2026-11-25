# typed: true
extend T::Sig

class A
end
AliasToA = A

sig {returns(AliasToA::B)}
#            ^^^^^^^^^^^ error: Unable to resolve constant `B`
def example; end
