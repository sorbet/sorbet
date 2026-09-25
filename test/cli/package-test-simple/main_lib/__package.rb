# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Project::MainLib < PackageSpec
  import Project::Util
  export Project::MainLib::Lib
end
