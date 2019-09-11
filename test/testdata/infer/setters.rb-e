# typed: true
class Foo
  extend T::Sig

  sig {params(a: String).returns(NilClass)}
  def foo=(a)
  end

  def bla
    self.foo = 1 # error: Assigning a value to `a` that does not match expected type `String`
  end
end
