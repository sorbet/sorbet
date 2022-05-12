# typed: strict

class Foo; end
class Bar < Foo; end
class Baz < Foo; end

# Ensure that order doesn't matter
class A
  extend T::Generic

  include M

  Args = type_member {{fixed: NilClass}}
  #                           ^^^^^^^^ error: The `fixed` type bound `NilClass` must be a supertype of the parent's `lower` type bound `Bar` for type_member `Args`
  #                           ^^^^^^^^ error: The `fixed` type bound `NilClass` must be a subtype of the parent's `upper` type bound `Foo` for type_member `Args`
end

# Ensure that type_templates depend on the singleton mixins
class B
  extend T::Generic
  extend M

  Args = type_template {{fixed: NilClass}}
  #                             ^^^^^^^^ error: The `fixed` type bound `NilClass` must be a supertype of the parent's `lower` type bound `Bar` for type_template `Args`
  #                             ^^^^^^^^ error: The `fixed` type bound `NilClass` must be a subtype of the parent's `upper` type bound `Foo` for type_template `Args`
end

module M
  extend T::Generic

  Args = type_member {{upper: Foo, lower: Bar}}
end

class C
  extend T::Generic

  include M

  Args = type_member {{upper:Foo, lower: Baz}}
  #                                      ^^^ error: The `lower` type bound `Baz` must be a supertype of the parent's `lower` type bound `Bar` for type_member `Args`
end
