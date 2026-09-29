# typed: strict

class Baz < PackageSpec
  # Puts `Baz` in a later stratum than `Opus::Foo`.
  import Opus::Foo
end
