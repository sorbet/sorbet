# typed: strict

class Outer
  A1 = T.let(0, Integer) # error: Cannot initialize the class or module `A1` by constant assignment
  module Inner; end
  A2 = Inner # error: Cannot initialize the class or module `A2` by constant assignment

  A1::B = T.let('', String)
  Inner::B = T.let('', String)
  A2::B = T.let('', String)
end
