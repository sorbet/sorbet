# typed: strict

class Project::Foo < PackageSpec
  import Project::Bar

  export Foo
  export_methods Foo
end
