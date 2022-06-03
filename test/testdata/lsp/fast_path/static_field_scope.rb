# typed: strict

class Outer
  A1 = T.let(0, Integer)
  module Inner; end
  A2 = Inner

  A1::B = T.let('', String) # error: Can't nest `B` under `Outer::A1` because `Outer::A1` is not a class or module
  Inner::B = T.let('', String)
  A2::B = T.let('', String) # error: Can't nest `B` under `Outer::A2` because `Outer::A2` is not a class or module
end
