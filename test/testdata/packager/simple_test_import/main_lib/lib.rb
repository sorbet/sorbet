# frozen_string_literal: true
# typed: strict

class Project::MainLib::Lib
  Project::Util::MyUtil.new

  # Normal code is not allowed to access names from `test_import`
  Project::TestOnly::SomeHelper.new
# ^^^^^^^^^^^^^^^^^ error: No import provides `Project::TestOnly`

  Test::Project::Util::UtilHelper
# ^^^^ error: Unable to resolve constant `Test`

  Test::Project::Util::Unexported
# ^^^^ error: Unable to resolve constant `Test`
end
