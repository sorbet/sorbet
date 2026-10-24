# typed: true

module Test::ExemptBehavior # error: `Test::ExemptBehavior` cannot define behavior, because `Test::ExemptBehavior` encloses the tests of packages nested under `ExemptBehavior`
  def self.test_helper; end
end

Test::ExemptBehavior.test_helper
