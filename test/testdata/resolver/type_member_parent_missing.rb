# typed: strict
# disable-fast-path: true

  class Foo1
# ^^^^^^^^^^ error: Type `Elem` declared by parent `Enumerable` must be re-declared in `Foo1`
# ^^^^^^^^^^ error: Missing definition for abstract method `Enumerable#each`
    include Enumerable
  end

  class Bar1 < Foo1
# ^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method `Enumerable#each`
    Elem = type_member(:out)
  # ^^^^^^^^^^^^^^^^^^^^^^^^ error: Classes can only have invariant type members
  # ^^^^^^^^^^^^^^^^^^^^^^^^ error: Type variance mismatch with parent `Foo1`
    #      ^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(Bar1)`
  end

  class Foo2
# ^^^^^^^^^^ error: Missing definition for abstract method `Enumerable#each`
    include Enumerable
    Elem = T.let(0, Integer)
  # ^^^^ error: Type variable `Elem` needs to be declared as `= type_member(SOMETHING)`
  end

  class Bar2 < Foo2
# ^^^^^^^^^^^^^^^^^ error: Type `Elem` declared by parent `Foo2` must be re-declared in `Bar2`
# ^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method `Enumerable#each`
    Elem = type_member(:out)
  # ^^^^^^^^^^^^^^^^^^^^^^^^ error: Classes can only have invariant type members
    #      ^^^^^^^^^^^^^^^^^ error: `Bar2::Elem` is a type member but `Foo2::Elem` is not a type member
    #      ^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(Bar2)`
  end
