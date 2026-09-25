# typed: strict

class Test::Foo::MissingImport < PackageSpec
  test!

  import Foo::MissingImport
  import Other
end
