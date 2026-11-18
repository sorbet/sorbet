# typed: strict

T::Descendants.register(Root::A::Parent, Root::B::Child, "test/testdata/packager/track_descendants/b/b.rb")
# A leading `./` is ignored.
T::Descendants.register(Root::B::Child, Root::B::GrandChild, "./test/testdata/packager/track_descendants/b/b.rb")
T::Descendants.register(Root::A::Mixin, Root::B::Includer, "test/testdata/packager/track_descendants/b/b.rb")
T::Descendants.register(Root::A::Parent, Root::B::WrongSuperclass, "test/testdata/packager/track_descendants/b/b.rb")

T::Descendants.register(Root::A::Parent, Root::B::Child) # error: `register` must be called with exactly 3 positional arguments
T::Descendants.register(Root::A::Parent, Root::B::Child, "test/testdata/packager/track_descendants/b/b.rb", "test/testdata/packager/track_descendants/b/b.rb") # error: `register` must be called with exactly 3 positional arguments
T::Descendants.register(Root::A::Parent, Root::B::Child, file: "test/testdata/packager/track_descendants/b/b.rb") # error: `register` must be called with exactly 3 positional arguments

T::Descendants.register(Root::A::Parent, Root::B::Child.itself, "test/testdata/packager/track_descendants/b/b.rb") # error: The first two arguments to `register` must be statically resolvable to classes or modules

T::Descendants.register(Root::A::Parent, Root::B::Child, :"test/testdata/packager/track_descendants/b/b.rb") # error: Expected a string file path as the third argument to `register`
T::Descendants.register(Root::A::Parent, Root::B::Child, "test/testdata/packager/track_descendants/b/" + "b.rb") # error: Expected a string file path as the third argument to `register`

T::Descendants.register(Root::A::Parent, Root::B::Child, "test/testdata/packager/track_descendants/b/descendants.rbi") # error: `Root::B::Child` is defined in `test/testdata/packager/track_descendants/b/b.rb`, not in `test/testdata/packager/track_descendants/b/descendants.rbi`

T::Descendants.register(Root::A::DoesNotExist, Root::B::Child, "test/testdata/packager/track_descendants/b/b.rb")
#                       ^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`
#                       ^^^^^^^^^^^^^^^^^^^^^ error: First argument to `register` must be statically resolvable to a class or module

T::Descendants.register(Root::A::NotAModule, Root::B::Child, "test/testdata/packager/track_descendants/b/b.rb")
#                       ^^^^^^^^^^^^^^^^^^^ error: First argument to `register` must be statically resolvable to a class or module

T::Descendants.register(Root::A::Parent, Root::A::NotAModule, "test/testdata/packager/track_descendants/a/a.rb")
#                                        ^^^^^^^^^^^^^^^^^^^ error: Second argument to `register` must be statically resolvable to a class or module

T::Descendants.register(Root::A::Parent, Root::B::DoesNotExist, "test/testdata/packager/track_descendants/b/b.rb")
#                                        ^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`
#                                        ^^^^^^^^^^^^^^^^^^^^^ error: Second argument to `register` must be statically resolvable to a class or module

# A descendant may only be registered from the package that defines it.
T::Descendants.register(Root::A::Parent, Root::A::LocalChild, "test/testdata/packager/track_descendants/a/a.rb")
#                                        ^^^^^^^^^^^^^^^^^^^ error: `Root::A::LocalChild` must be tracked in its own package

T::Descendants.register(Root::A::Parent, Root::B::Plain, "test/testdata/packager/track_descendants/b/b.rb") # error: `Root::B::Plain` is not a descendant of `Root::A::Parent`
