# frozen_string_literal: true
# typed: strict

class Project::Foo < PackageSpec
  import Project::Bar

  export Project::Foo::FooClass
  export Project::Foo::FooMethods
end
