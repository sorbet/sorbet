# typed: strict

module Wrapper
  extend T::Generic

  class A; end
  A = 1 # error: Cannot initialize the class `A` by constant assignment
  A = type_member # error: Redefining constant `A` as a type member or type template

  B = 1 # error: Cannot initialize the class `B` by constant assignment
  class B; end
  B = type_member # error: Redefining constant `B` as a type member or type template

  C = 1 # error: Cannot initialize the class `C` by constant assignment
  C = type_member # error: Redefining constant `C` as a type member or type template
  class C; end

  class D; end
  D = type_member # error: Redefining constant `D` as a type member or type template
  D = 1 # error: Cannot initialize the class `D` by constant assignment

  E = type_member # error: Redefining constant `E` as a type member or type template
  class E; end
  E = 1 # error: Cannot initialize the class `E` by constant assignment

  F = type_member # error: Redefining constant `F` as a type member or type template
  F = 1 # error: Cannot initialize the class `F` by constant assignment
  class F; end
end
