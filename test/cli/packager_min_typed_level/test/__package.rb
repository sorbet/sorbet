# typed: strict

class Test::Project::MyPackage < PackageSpec
  test!

  import Project::MyPackage, uses_internals: true
end
