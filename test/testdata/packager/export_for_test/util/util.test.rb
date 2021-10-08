# typed: strict

module Test::Opus::Util
  class TestUtil; end

  # Available via `export_for_test Opus::Util::UtilClass::Nesting`
  Opus::Util::Nesting.nesting_method
  Opus::Util::Nesting::Public.public_method
end
