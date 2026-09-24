# frozen_string_literal: true
# typed: strict

module Test::Outer
  class OK; end

  MY_CONST = 1

  module Inner
#        ^^^^^ error: Tests in the `Outer` package must define tests in the `Test::Outer` namespace
    module Foo; end
  end

  module Inner::Bar; end
# ^^^^^^^^^^^^^^^^^ error: Package `Outer` may not open `Test::Outer::Inner::Bar`
#        ^^^^^^^^^^ error: Tests in the `Outer` package must define tests in the `Test::Outer` namespace
end
