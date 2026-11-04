# frozen_string_literal: true
# typed: strict

class Project::MainLib::Lib
  Project::Util::MyUtil.new

  Project::TestOnly::SomeHelper.new
# ^^^^^^^^^^^^^^^^^ error: `Project::TestOnly` is not imported

  Test::Project::Util::UtilHelper
# ^^^^^^^^^^^^^^^^^^^ error: `Test::Project::Util` cannot be referenced here because `Project::MainLib` may not reference `test!` packages

  Test::Project::Util::Unexported
# ^^^^^^^^^^^^^^^^^^^ error: `Test::Project::Util` cannot be referenced here because `Project::MainLib` may not reference `test!` packages
end
