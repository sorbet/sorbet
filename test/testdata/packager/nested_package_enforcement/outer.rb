# frozen_string_literal: true
# typed: strict

module Outer
  class OK; end

  MY_CONST = 1

  module Inner
#        ^^^^^ error: File belongs to package `Outer` but defines a constant that does not match this namespace
    module Foo; end
  end

  # `Inner` is looked up in `Outer`, which this package owns, and nothing is resolved inside `Inner::Bar`, so the
  # only error here is the namespace mismatch on the definition itself.
  module Inner::Bar; end
#        ^^^^^^^^^^ error: File belongs to package `Outer` but defines a constant that does not match this namespace
end
