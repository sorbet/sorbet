# typed: strict
# enable-packager: true

class Opus::Foo < PackageSpec
  # Puts `Opus::Foo::Early` in an earlier stratum than `Opus::Foo`.
  import Opus::Foo::Early
end
