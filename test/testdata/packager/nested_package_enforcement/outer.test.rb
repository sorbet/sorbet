# frozen_string_literal: true
# typed: strict

module Test::Outer
#      ^^^^^^^^^^^ error: File belongs to package `Outer` but defines a constant that does not match this namespace
  class OK; end

  MY_CONST = 1

  module Inner
    module Foo; end
  end

  module Inner::Bar; end
end
