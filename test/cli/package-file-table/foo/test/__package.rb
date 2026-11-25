# frozen_string_literal: true
# typed: strict

class Test::Project::Foo < PackageSpec
  test!

  import Project::Foo, uses_internals: true
end
