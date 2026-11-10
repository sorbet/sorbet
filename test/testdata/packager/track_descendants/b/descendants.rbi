# typed: strict

T::Descendants.register(Root::A::Parent, Root::B::Child)
T::Descendants.register(Root::B::Child, Root::B::GrandChild)
T::Descendants.register(Root::A::Mixin, Root::B::Includer)
T::Descendants.register(Root::A::Parent, Root::B::WrongSuperclass)

T::Descendants.register(Root::A::Parent) # error: `register` must be called with exactly 2 positional arguments
T::Descendants.register(Root::A::Parent, Root::B::Child, Root::B::Child) # error: `register` must be called with exactly 2 positional arguments
T::Descendants.register(Root::A::Parent, subclass: Root::B::Child) # error: `register` must be called with exactly 2 positional arguments

T::Descendants.register(Root::A::Parent, Root::B::Child.itself) # error: Arguments to `register` must be statically resolvable to classes or modules

T::Descendants.register(Root::A::NotAModule, Root::B::Child)
#                       ^^^^^^^^^^^^^^^^^^^ error: First argument to `register` must be statically resolvable to a class or module

T::Descendants.register(Root::A::Parent, Root::A::NotAModule)
#                                        ^^^^^^^^^^^^^^^^^^^ error: Second argument to `register` must be statically resolvable to a class or module

T::Descendants.register(Root::A::Parent, Root::B::DoesNotExist)
#                                        ^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`
#                                        ^^^^^^^^^^^^^^^^^^^^^ error: Second argument to `register` must be statically resolvable to a class or module

# A descendant may only be registered from the package that defines it.
T::Descendants.register(Root::A::Parent, Root::A::LocalChild)
#                                        ^^^^^^^^^^^^^^^^^^^ error: `Root::A::LocalChild` must be tracked in its own package

T::Descendants.register(Root::A::Parent, Root::B::Plain) # error: `Root::B::Plain` is not a descendant of `Root::A::Parent`
