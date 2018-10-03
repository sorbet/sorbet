# typed: true
class A
  B = 1
end

A::B::C = 1 # this _should_ be an error but isn't yet
A::B::D # error: Unable to resolve constant
A::E # error: Unable to resolve constant
