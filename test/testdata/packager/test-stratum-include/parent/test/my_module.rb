# typed: strict
# frozen_string_literal: true

# stratum: 2

module Parent::TestMixin # error: Tests in the `Parent` package must define tests in the `Test::Parent` namespace
end
