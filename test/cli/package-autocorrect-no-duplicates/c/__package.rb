# typed: strict

class C < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'

  export C::Foo
end
