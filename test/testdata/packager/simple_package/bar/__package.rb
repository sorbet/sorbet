# typed: strict

class Project::Bar < PackageSpec
  import Project::Foo

  export Bar
  export_methods Bar
end
