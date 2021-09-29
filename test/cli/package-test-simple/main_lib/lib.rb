# frozen_string_literal: true
# typed: strict

class Project::MainLib::Lib
  Project::Util::MyUtil.new

  # ERROR: Normal code is not allowed to access names from `test_import`
  # TODO(ngroman) Move this error enforcement into Sorbet
  Project::TestOnly::SomeHelper.new
end
