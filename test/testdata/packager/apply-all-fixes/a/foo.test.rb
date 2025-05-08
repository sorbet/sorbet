# typed: true

class Test::A::FooTest
  C::Foo.new # error: `C::Foo` resolves but its package is not imported
end
