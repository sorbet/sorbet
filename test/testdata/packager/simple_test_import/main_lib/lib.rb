# frozen_string_literal: true
# typed: strict

class Project::MainLib::Lib
  Project::Util::MyUtil.new

  Project::TestOnly::SomeHelper.new
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Project::TestOnly::SomeHelper` resolves but its package is not imported

  Test::Project::Util::UtilHelper
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Project::Util::UtilHelper` cannot be referenced here because `Project::MainLib` may not reference `test!` packages

  Test::Project::Util::Unexported
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Project::Util::Unexported` cannot be referenced here because `Project::MainLib` may not reference `test!` packages
end
