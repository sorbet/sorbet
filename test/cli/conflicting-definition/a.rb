require 'x'

module Foo
   def meth; end # Defines behavior
end

module Bar
  module BarInner
    1 # Defines behavior
  end
end
