# typed: strict
extend T::Sig

class MyEnum
  extend T::Sig
  extend T::Helpers
  sealed!
  abstract!

  sig {params(x: MyEnum).void}
  def self.foo(x)
  end
end
