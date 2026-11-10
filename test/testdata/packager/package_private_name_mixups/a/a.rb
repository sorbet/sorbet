# typed: strict

module Root::A
  class Example
    extend T::Sig

    SomeConstant = 1
    class SomeClass; end

    sig { void }
    def some_method; end

    sig { void }
    def self.some_class_method; end

    # Constants, passed to `package_private`.
    package_private :SomeConstant
    #               ^^^^^^^^^^^^^ error: `package_private` can only make methods package-private, but `SomeConstant` is a constant
    package_private(:SomeConstant)
    #               ^^^^^^^^^^^^^ error: `package_private` can only make methods package-private, but `SomeConstant` is a constant
    package_private :SomeConstant, :some_method
    #               ^^^^^^^^^^^^^ error: `package_private` can only make methods package-private, but `SomeConstant` is a constant
    # No autocorrect: a class is marked by `package_private!` in its own body, not by name.
    package_private :SomeClass
    #               ^^^^^^^^^^ error: `package_private` can only make methods package-private, but `SomeClass` is a constant

    # Methods, passed to the constant modifier.
    package_private!(:some_method)
    #                ^^^^^^^^^^^^ error: `package_private!` can only make constants package-private, but `some_method` is a method
    package_private!(:some_class_method)
    #                ^^^^^^^^^^^^^^^^^^ error: `package_private!` can only make constants package-private, but `some_class_method` is a method

    # Names that match nothing.
    package_private :does_not_exist
    package_private!(:DoesNotExist)
    #                ^^^^^^^^^^^^^ error: `package_private!` can only mark a constant defined in the same file
  end
end
