# typed: strict
# frozen_string_literal: true

# stratum: 2

class Parent::MyGeneric # error: Tests in the `Parent` package must define tests in the `Test::Parent` namespace
  Elem2 = type_member
  #       ^^^^^^^^^^^ error: `type_member` in a test file cannot modify a non-test class in the same package
end
