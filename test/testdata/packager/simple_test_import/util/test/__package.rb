# frozen_string_literal: true
# typed: strict

class Test::Project::Util < PackageSpec
  test!

  import Project::Util

  export Test::Project::Util::UtilHelper
end
