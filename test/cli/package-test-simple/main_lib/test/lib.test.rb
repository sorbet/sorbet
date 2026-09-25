# frozen_string_literal: true
# typed: strict

class Test::Project::MainLib::LibTest
  # Tests can access their package's code via uses_internals
  Project::MainLib::Lib.new
  # Tests can access `import`
  Project::Util::MyUtil.new
  # Tests can access imports on the test package
  Project::TestOnly::SomeHelper.new
end

# Add "behavior" to this file. When enforcing `MultipleBehaviorDefs` in
# `--uniquely-defined-behavior` the top-level of <PackageTests> should be ignored.
1 + 1
