# typed: strict

class Foo::Bar::FalseAndAppPackage < PackageSpec
  layer 'app'
  strict_dependencies 'false'
end
