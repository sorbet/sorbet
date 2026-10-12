# typed: strict
# stratum: 1

class A < PackageSpec
  import B
  export A::Thing
  export A::Same
# ^^^^^^^^^^^^^^ error: Constant `A::Same` lacks a declaration in this package and cannot be exported
  export A::Foreign
# ^^^^^^^^^^^^^^^^^ error: Constant `A::Foreign` lacks a declaration in this package and cannot be exported
  export A::Chained
# ^^^^^^^^^^^^^^^^^ error: Constant `A::Chained` lacks a declaration in this package and cannot be exported
  export A::ModuleAlias
# ^^^^^^^^^^^^^^^^^^^^^ error: Constant `A::ModuleAlias` lacks a declaration in this package and cannot be exported
  export A::Unpackaged
# ^^^^^^^^^^^^^^^^^^^^ error: Constant `A::Unpackaged` lacks a declaration in this package and cannot be exported
end
