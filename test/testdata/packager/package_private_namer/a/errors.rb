# typed: strict

module Root::A
  package_private!
# ^^^^^^^^^^^^^^^^ error: `package_private!` cannot mark the root namespace of package `Root::A`

  module Nested
    package_private!
  # ^^^^^^^^^^^^^^^^ error: `package_private!` cannot mark `Root::A::Nested`, because it encloses another package
  end

  class Errors
    extend T::Generic

    class Inner; end
    package_private!(:Inner)
    #                ^^^^^^ error: Use `package_private!` in the body of `Root::A::Errors::Inner` to mark it package-private

    package_private!(:Missing)
    #                ^^^^^^^^ error: `package_private!` can only mark a constant defined in the same file

    # Defined in another class.
    package_private!(:Y)
    #                ^^ error: `package_private!` can only mark a constant defined in the same file

    Elem = type_member
    package_private!(:Elem)
    #                ^^^^^ error: `package_private!` can only mark a constant, type alias, or class alias

    package_private!(:A, :B)
  # ^^^^^^^^^^^^^^^^^^^^^^^^ error: `package_private!` takes no arguments, or the name of one constant as a Symbol
  #                      ^^ error: Too many arguments provided for method `Module#package_private!`

    package_private!("Elem")
    #                ^^^^^^ error: `package_private!` takes no arguments, or the name of one constant as a Symbol
    #                ^^^^^^ error: Expected `T.nilable(Symbol)` but found `String("Elem")` for argument `name`

    class << self
      package_private!
    end
  end

  class Marked
    # Defined in this class, but in another file.
    package_private!(:Alias)
    #                ^^^^^^ error: `package_private!` can only mark a constant defined in the same file
  end
end
