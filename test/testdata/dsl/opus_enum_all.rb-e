# typed: strict
extend T::Sig

module Opus
  class Enum
    extend T::Generic
  end
end

class MyEnum < Opus::Enum
  A = new
  B = new
end

# It used to be the case that this caused "unsupported usage of bare type",
# because we were assuming that we'd be able to unwrap the type.
sig {params(x: T.all(MyEnum::A, MyEnum::B)).void}
def foo(x); end
