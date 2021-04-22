# frozen_string_literal: true
# typed: strict

class Project::Foo < PackageSpec
  import Project::Bar

  export Foo
  export FooMethods
end
