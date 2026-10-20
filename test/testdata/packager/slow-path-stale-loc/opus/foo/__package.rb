# typed: strict
# enable-packager: true

class Opus::Foo < PackageSpec
  # Puts `Opus::Foo::Baz::Qux` in an earlier stratum than `Opus::Foo`.
  import Opus::Foo::Baz::Qux
end
