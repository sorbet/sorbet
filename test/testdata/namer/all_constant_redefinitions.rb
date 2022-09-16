# typed: strict
# disable-fast-path: true

module Wrapper
  extend T::Generic

  class A; end
  A = 1 # error: Redefining constant `A` as a static field
  A = type_member # error: Redefining constant `A` as a type member or type template

  B = 1
  class B; end # error: Redefining constant `B` as a class or module
  B = type_member # error: Redefining constant `B` as a type member or type template

  C = 1
  C = type_member # error: Redefining constant `C` as a type member or type template
  class C; end # error: Redefining constant `C` as a class or module

  class D; end
  D = type_member # error: Redefining constant `D` as a type member or type template
  D = 1 # error: Redefining constant `D` as a static field

  E = type_member
  class E; end  # error: Redefining constant `E` as a class or module
  E = 1 # error: Redefining constant `E` as a static field

  F = type_member
  F = 1 # error: Redefining constant `F` as a static field
  class F; end # error: Redefining constant `F` as a class or module
end
