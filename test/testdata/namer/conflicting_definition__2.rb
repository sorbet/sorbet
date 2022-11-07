# typed: false
require 'y' # root level 'behavior' is ignored

module Foo
   def meth1; end # Defines behavior
end

module Bar
  # Constant assignment is not considered "defining behavior" for Bar
  X = 1

  module BarInner; end
end
