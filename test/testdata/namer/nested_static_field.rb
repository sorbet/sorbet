# typed: false
class A
  B = T.unsafe(nil) # error: Cannot initialize the class or module `B` by constant assignment
  class B::C; end

  E = T.unsafe(nil)
  E::F = T.unsafe(nil) # error: Can't nest `F` under `A::E` because `A::E` is not a class or module

  B::C # error: Unable to resolve constant `C`
  E::F
end
