# frozen_string_literal: true

# typed: strict

class Project::Foo < PackageSpec
  export Project::Foo::B
  export Project::Foo::D
end
