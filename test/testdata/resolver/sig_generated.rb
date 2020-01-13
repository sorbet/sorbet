# typed: true

extend T::Sig

sig {returns(NilClass).generated} # error: Malformed signature
def generated
end
