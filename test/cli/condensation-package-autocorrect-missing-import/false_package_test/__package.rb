# typed: strict

class Foo::Bar::FalsePackageTest < PackageSpec
  layer 'lib'
  strict_dependencies 'false'
end
