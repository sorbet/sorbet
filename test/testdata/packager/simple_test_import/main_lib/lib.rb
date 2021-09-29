# frozen_string_literal: true
# typed: strict

class Project::MainLib::Lib
  Project::Util::MyUtil.new

  # Normal code is not allowed to access names from `test_import`
  # TODO TODO this should not be allowed
  Project::TestOnly::SomeHelper.new

  Test
# ^^^^ error: Unable to resolve constant `Test`

  Test::Project::Util::UtilHelper
# ^^^^ error: Unable to resolve constant `Test`

  Test::Project::Util::Unexported
# ^^^^ error: Unable to resolve constant `Test`
end
