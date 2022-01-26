# typed: strict

module Test::Root
  BAD = 1
# ^^^ error: Constants may not be defined outside of the enclosing package namespace `Test::Root::Nested`
  #
  module Nested
    class Ok; end

    OK_CONSTANT = 1
  end
end

module Root::Nested::Foo
#      ^^^^^^^^^^^^^^^^^ error: Class or method definition must match enclosing package namespace `Test::Root::Nested`
end
