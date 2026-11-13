# typed: strict

class Foo::Bar::FalsePackage < PackageSpec
  layer 'lib'
  strict_dependencies 'false'
end
