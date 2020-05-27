# typed: true
class A
  B = 1
end

A::B::C = 1 # error: Can't nest `C` under `A::B` because `A::B` is not a class or module
A::B::D # error: Unable to resolve constant
A::E # error: Unable to resolve constant
