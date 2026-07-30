# frozen_string_literal: true
# typed: strict

class Test::Project::MainLib::LibTest
  Project::MainLib::Lib.new
  Project::Util::MyUtil.new
  Project::TestOnly::SomeHelper.new

  Test::Project::Util::UtilHelper # allowed by import

  Test::Project::Util::Unexported
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Project::Util::Unexported` resolves but is not exported from `Test::Project::Util`
end
