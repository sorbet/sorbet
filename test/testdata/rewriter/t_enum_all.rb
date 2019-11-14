# typed: strict
extend T::Sig

class MyEnum < T::Enum
  enums do
  A = new
  B = new
  end
end

# It used to be the case that this caused "unsupported usage of bare type",
# because we were assuming that we'd be able to unwrap the type.
sig {params(x: T.all(MyEnum::A, MyEnum::B)).void}
def foo(x); end
