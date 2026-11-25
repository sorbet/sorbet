# frozen_string_literal: true
# typed: strict

class Project::MainLib::Lib
  Project::Util::MyUtil.new

  # ERROR: Normal code cannot access test-only packages
  Project::TestOnly::SomeHelper.new
end
