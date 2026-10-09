# typed: strict

class A < PackageSpec
  import B
  export A::Thing
  export A::Same
  export A::Foreign
  export A::Chained
  export A::ModuleAlias
  export A::Unpackaged
end
