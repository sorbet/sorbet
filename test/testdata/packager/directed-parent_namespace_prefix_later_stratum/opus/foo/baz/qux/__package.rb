# typed: strict

class Opus::Foo::Baz::Qux < PackageSpec
  # Puts `Opus::Foo::Baz::Qux` in a later stratum than `Opus::Foo::Bar`.
  import Opus::Foo::Bar
end
