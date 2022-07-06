# frozen_string_literal: true

# typed: strict

class Project::Baz::Package < PackageSpec
  export Project::Baz::Package::C
  export Project::Baz::Package::E
end
