# typed: strict

class Foo::MissingImport < PackageSpec
  test_import Other, only: "test_rb"
end
