# typed: strict

module Test::Root
  BAD = 1
# ^^^ error: File belongs to package `Test::Root::Nested` but defines a constant that does not match this namespace
  #
  module Nested
    class Ok; end

    OK_CONSTANT = 1
  end
end

module Root::Nested::Foo
#      ^^^^^^^^^^^^^^^^^ error: File belongs to package `Test::Root::Nested` but defines a constant that does not match this namespace
end
