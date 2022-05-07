# frozen_string_literal: true
# typed: strict

module Outer
  class OK; end

  MY_CONST = 1

  module Inner
#        ^^^^^ error: File belongs to package `Outer` but defines a constant that does not match this namespace
    module Foo; end
  end

  module Inner::Bar; end
#        ^^^^^^^^^^ error: File belongs to package `Outer` but defines a constant that does not match this namespace
end
