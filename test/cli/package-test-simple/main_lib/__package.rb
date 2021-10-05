# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Project::MainLib < PackageSpec
  import Project::Util
  test_import Project::TestOnly
  export_for_test Project::MainLib::Lib
end
