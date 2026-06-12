# typed: strict

class Opus::Foo < PackageSpec
  import Opus::Foo::Bar
  import Opus::Util

  export Opus::Foo::FooClass
end
