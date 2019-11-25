# typed: true

extend T::Sig

sig {returns(NilClass).generated} # error: `generated` is deprecated
def generated
end
