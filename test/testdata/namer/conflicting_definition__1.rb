# typed: false
# stripe-mode: true
require 'x'

module Foo # error: `Foo` has behavior defined in multiple files
   def meth; end # Defines behavior
end

module Bar
  module BarInner
    1 # Defines behavior
  end
end
