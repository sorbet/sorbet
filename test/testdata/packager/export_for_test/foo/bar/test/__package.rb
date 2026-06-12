# typed: strict

class Test::Opus::Foo::Bar < PackageSpec
  test!

  import Opus::Foo::Bar

  export Test::Opus::Foo::Bar::BarClassTest
end
