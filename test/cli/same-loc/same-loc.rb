# typed: true

class MyClass < Foo::Bar
  const :foo, T::Array[Foo::Baz]
end
