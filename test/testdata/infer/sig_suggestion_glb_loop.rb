# typed: strict

module Parent; end
module Child
  include Parent
end

class Unrelated; end

class A
  extend T::Sig

  sig { params(x: Unrelated).void }
  def foo1(x); end
  sig { params(x: Parent).void }
  def foo2(x); end
  sig { params(x: Child).void }
  def foo3(x); end

  def test(x) # error: does not have a `sig`
    foo1(x)
    foo2(x)

    1.times do
      foo3(x)
    end
  end
end
