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
    package_private(:SomeConstant)
    package_private :SomeConstant, :some_method
    package_private :SomeClass

    # Methods, passed to the constant modifier.
    package_private!(:some_method)
    #                ^^^^^^^^^^^^ error: `package_private!` can only mark a constant defined in the same file
    package_private!(:some_class_method)
    #                ^^^^^^^^^^^^^^^^^^ error: `package_private!` can only mark a constant defined in the same file

    # Names that match nothing.
    package_private :does_not_exist
    package_private!(:DoesNotExist)
    #                ^^^^^^^^^^^^^ error: `package_private!` can only mark a constant defined in the same file
  end
end
