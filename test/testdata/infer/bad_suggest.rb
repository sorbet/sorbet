# typed: true
class A
  B = 1 # error: Cannot initialize the class or module `B` by constant assignment
end

A::B::C = 1
A::B::D # error: Unable to resolve constant
A::E # error: Unable to resolve constant
