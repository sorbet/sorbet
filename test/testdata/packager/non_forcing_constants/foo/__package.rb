# frozen_string_literal: true
# typed: strict

class Project::Foo < PackageSpec
  export Project::Foo::Foo
  export Project::Foo::FooNonForcing
end
