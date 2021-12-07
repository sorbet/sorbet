# typed: strict

class Foo::BasePkg < PackageSpec

  export Foo::BasePkg::Exists

  export Foo::BasePkg::WildlyMisspelled
  export Foo::BasePkg::NameWithTipo
end
