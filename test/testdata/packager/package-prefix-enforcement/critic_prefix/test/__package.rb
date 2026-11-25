# typed: strict

class Test::Critic::SomePkg < PackageSpec
  test!
  import Critic::SomePkg, uses_internals: true
end
