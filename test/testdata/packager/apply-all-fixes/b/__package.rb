# typed: strict

class B < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'

  export B::Foo
end
