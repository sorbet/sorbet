# frozen_string_literal: true
# typed: strict

class Project::MainLib::Lib
  Project::Util::MyUtil.new

  # Normal code is not allowed to access names from `test_import`
  Project::TestOnly::SomeHelper.new
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Used `test_import` constant `Project::TestOnly::SomeHelper` in non-test file

  Test::Project::Util::UtilHelper
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Used test-only constant `Test::Project::Util::UtilHelper` in non-test file

  Test::Project::Util::Unexported
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Package `Project::Util` does not export `Test::Project::Util::Unexported`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Used test-only constant `Test::Project::Util::Unexported` in non-test file
end
