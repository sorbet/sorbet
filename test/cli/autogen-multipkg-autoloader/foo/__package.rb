# typed: strict

class Project::Foo < PackageSpec
  import Project::Bar

  export Project::Foo::FooClass
end
