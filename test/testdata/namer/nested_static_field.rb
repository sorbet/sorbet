# typed: false
class A
  B = T.unsafe(nil) # error: Cannot initialize the class or module `B` by constant assignment
  class B::C; end

  E = T.unsafe(nil) # error: Cannot initialize the class or module `E` by constant assignment
  E::F = T.unsafe(nil)

  B::C
  E::F
end
