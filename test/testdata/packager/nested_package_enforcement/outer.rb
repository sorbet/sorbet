# frozen_string_literal: true
# typed: strict

module Outer
  class OK; end

  MY_CONST = 1

  module Inner
#        ^^^^^ error: Class or method definition must match enclosing package namespace `Outer`
    module Foo; end
  end

  module Inner::Bar; end
#        ^^^^^^^^^^ error: Class or method definition must match enclosing package namespace `Outer`
end
