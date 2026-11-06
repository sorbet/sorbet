# typed: strict

module Root::A
  package_private!

  module Nested
    package_private!
  end

  class Errors
    extend T::Generic

    class Inner; end
    package_private!(:Inner)

    package_private!(:Missing)

    # Defined in another class.
    package_private!(:Y)

    Elem = type_member
    package_private!(:Elem)

    package_private!(:A, :B)
  #                      ^^ error: Too many arguments provided for method `Module#package_private!`

    package_private!("Elem")
    #                ^^^^^^ error: Expected `T.nilable(Symbol)` but found `String("Elem")` for argument `name`

    class << self
      package_private!
    end
  end

  class Marked
    # Defined in this class, but in another file.
    package_private!(:Alias)
  end
end
