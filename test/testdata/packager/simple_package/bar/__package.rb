# typed: strict
# enable-packager: true

class Project::Bar < PackageSpec
  import Project::Foo

  export Bar
  export_methods Bar
end
