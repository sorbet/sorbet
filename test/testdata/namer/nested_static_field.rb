# typed: false
# disable-fast-path: true
class A
  B = T.unsafe(nil)
  class B::C; end # error: Can't nest `C` under `A::B` because `A::B` is not a class or module

  E = T.unsafe(nil)
  E::F = T.unsafe(nil) # error: Can't nest `F` under `A::E` because `A::E` is not a class or module

  B::C
  E::F
end
