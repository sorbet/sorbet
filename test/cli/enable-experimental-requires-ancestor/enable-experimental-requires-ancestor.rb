# typed: true

module Foo
  extend T::Helpers

  requires_ancestor { Kernel }
end

class Bar < BasicObject
  include Foo
end
