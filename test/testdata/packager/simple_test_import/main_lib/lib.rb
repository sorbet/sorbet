# frozen_string_literal: true
# typed: strict

class Project::MainLib::Lib
  Project::Util::MyUtil.new

  # Normal code is not allowed to access names from `test_import`
  Project::TestOnly::SomeHelper.new
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Used `test_import` constant `Project::TestOnly::SomeHelper` in non-test file

  Test::Project::Util::UtilHelper
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Project::Util::UtilHelper` is defined in a test namespace

  Test::Project::Util::Unexported
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Project::Util::Unexported` is defined in a test namespace
end
