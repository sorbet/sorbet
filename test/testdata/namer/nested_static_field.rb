# typed: false

class A
  B = T.unsafe(nil)
  class B::C; end # error: Can't nest `C` under `A::B` because `A::B` is not a class or module

  E = T.unsafe(nil)
  E::F = T.unsafe(nil) # error: Can't nest `F` under `A::E` because `A::E` is not a class or module

  # TODO(jez) For some reason, we handle these differently.
  # I don't know that it matters too much (because the code errors anyways)
  # but I figure it's good to at least document.
  B::C # error: Unable to resolve constant `C`
  E::F
end
