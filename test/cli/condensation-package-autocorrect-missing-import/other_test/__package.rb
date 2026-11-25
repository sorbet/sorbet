# typed: strict

class Foo::Bar::OtherPackageTest < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'
end
