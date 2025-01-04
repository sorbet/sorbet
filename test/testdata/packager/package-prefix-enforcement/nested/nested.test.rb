# typed: strict

module Test::Root
  BAD = 1
# ^^^ error: Tests in the `Root::Nested` package must define tests in the `Test::Root::Nested` namespace
# ^^^ error: `Test::Root::BAD` resolves but is not exported from `Root`
  #
  module Nested
    class Ok; end

    OK_CONSTANT = 1
  end
end

module Root::Nested::Foo
#      ^^^^^^^^^^^^^^^^^ error: Tests in the `Root::Nested` package must define tests in the `Test::Root::Nested` namespace
end
