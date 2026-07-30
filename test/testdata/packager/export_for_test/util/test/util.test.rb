# typed: strict

module Test::Opus::Util
  class TestUtil; end

  # Available via uses_internals: true
  Opus::Util::Nesting.nesting_method
  Opus::Util::Nesting::Public.public_method
end
