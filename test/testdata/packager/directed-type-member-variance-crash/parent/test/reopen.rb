# typed: strict
# frozen_string_literal: true

# stratum: 2

class Parent::MyGeneric # error: Tests in the `Parent` package must define tests in the `Test::Parent` namespace
  Elem2 = type_member # error: Tests in the `Parent` package must define tests in the `Test::Parent` namespace
  #       ^^^^^^^^^^^ error: `type_member` may only be used on constants in the package that owns them
end
