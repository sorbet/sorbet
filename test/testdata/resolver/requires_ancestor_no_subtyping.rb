# typed: true
# enable-experimental-requires-ancestor: true

module RA
  def foo; end
end

### Requiring ancestors does not create subtyping relationship

module Test1
  extend T::Sig

  module M1
    extend T::Helpers
    requires_ancestor { RA }
  end

  class C1
    include RA
    include M1
  end

  sig { params(c1: C1).void }
  def bar(c1)
    c1.foo
  end

  sig { returns(M1) }
  def m1
    C1.new
  end

  sig { returns(C1) }
  def c1
    C1.new
  end

  def main
    bar(m1) # error: Expected `Test1::C1` but found `Test1::M1` for argument `c1`
    bar(c1)
  end
end
