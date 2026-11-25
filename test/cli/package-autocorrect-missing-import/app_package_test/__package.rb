# typed: strict

class Foo::Bar::AppPackageTest < PackageSpec
  layer 'app'
  strict_dependencies 'layered'
end
