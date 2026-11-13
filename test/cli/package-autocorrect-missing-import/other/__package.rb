# typed: strict

class Foo::Bar::OtherPackage < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'
end
