# frozen_string_literal: true
# typed: strict

class Test::Project::MainLib < PackageSpec
  test!

  import Project::MainLib, uses_internals: true
  import Project::Util
  import Project::TestOnly
  import Test::Project::Util
end
