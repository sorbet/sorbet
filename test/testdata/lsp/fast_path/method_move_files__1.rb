# typed: true

class A
  extend T::Sig
end

T.reveal_type(A.new.m) # error: Revealed type: `Integer`
