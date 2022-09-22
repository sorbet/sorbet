# typed: strict

module Wrapper
  extend T::Generic
  C = 1 # error: Cannot initialize the class `C` by constant assignment
  C = type_member # error: Redefining constant `C` as a type member or type template
  class C; end
end
