# typed: true

module IFoo
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def foo; end

  # This redefinition is an error, but since redefinition errors are only
  # reported at `typed: true`, this situation might not have been reported to
  # the user.
  sig {abstract.params(x: Integer).void}
  def foo(x); end
# ^^^^^^^^^^ error: Method `IFoo#foo` redefined without matching argument count. Expected: `0`, got: `1`
end

  class FooGood
    include IFoo

    def foo(x); end
  end

  class FooBad
# ^^^^^^^^^^^^ error: Missing definition for abstract method `IFoo#foo`
    include IFoo
  end
