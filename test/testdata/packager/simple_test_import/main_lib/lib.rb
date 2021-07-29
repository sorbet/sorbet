# frozen_string_literal: true
# typed: strict

class Project::MainLib::Lib
  Project::Util::MyUtil.new

  # Normal code is not allowed to access names from `test_import`
  Project::TestOnly::SomeHelper.new
# ^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `TestOnly`
end
