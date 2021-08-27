# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Project::TestOnly < PackageSpec
  export Project::TestOnly::SomeHelper
end
