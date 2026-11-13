# typed: strict

class Foo::Bar::AppPackage < PackageSpec
  layer 'app'
  strict_dependencies 'layered'
end
