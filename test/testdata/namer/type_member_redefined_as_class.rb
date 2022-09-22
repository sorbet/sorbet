# typed: strict

module Wrapper
  extend T::Generic
  C = 1
  C = type_member # error: Redefining constant `C` as a type member or type template
  class C; end # error: Redefining constant `C` as a class or module
end
