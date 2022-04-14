# typed: strict
# disable-fast-path: true

class Foo; end
class Bar < Foo; end
class Baz < Foo; end

# Ensure that order doesn't matter
class A
  extend T::Generic

  include M

  Args = type_member {{fixed: NilClass}}
       # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: parent lower bound `Bar` is not a subtype of lower bound `NilClass`
       # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: upper bound `NilClass` is not a subtype of parent upper bound `Foo`
end

# Ensure that type_templates depend on the singleton mixins
class B
  extend T::Generic
  extend M

  Args = type_template {{fixed: NilClass}}
       # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: parent lower bound `Bar` is not a subtype of lower bound `NilClass`
       # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: upper bound `NilClass` is not a subtype of parent upper bound `Foo`
end

module M
  extend T::Generic

  Args = type_member {{upper: Foo, lower: Bar}}
end

class C
  extend T::Generic

  include M

  Args = type_member {{upper:Foo, lower: Baz}}
       # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: parent lower bound `Bar` is not a subtype of lower bound `Baz`
end
