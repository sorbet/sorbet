# frozen_string_literal: true
# typed: strict

class Test::Project::MainLib::LibTest
  # Tests can access their package's code if exported for test
  Project::MainLib::Lib.new
  # Tests can access `import`
  Project::Util::MyUtil.new
  # Tests can access `test_import` names
 Test::Project::Util::UtilHelper

 Project::TestOnly::SomeHelper.new # access via test_import
end

# Add "behavior" to this file. When enforcing `MultipleBehaviorDefs` in
# `--uniquely-defined-behavior` the top-level of <PackageTests> should be ignored.
1 + 1
