# typed: strict

T::Descendants.register(
  Root::A::Parent,
  {
    Root::B::Child => "test/testdata/packager/track_descendants/b/b.rb",
    Root::B::WrongSuperclass => "test/testdata/packager/track_descendants/b/b.rb",
    Root::B::Child.itself => "test/testdata/packager/track_descendants/b/b.rb",
  # ^^^^^^^^^^^^^^^^^^^^^ error: Subclasses passed to `register` must be statically resolvable to classes or modules
    Root::A::NotAModule => "test/testdata/packager/track_descendants/a/a.rb",
  # ^^^^^^^^^^^^^^^^^^^ error: Subclasses passed to `register` must be statically resolvable to classes or modules
    Root::B::DoesNotExist => "test/testdata/packager/track_descendants/b/b.rb",
  # ^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`
  # ^^^^^^^^^^^^^^^^^^^^^ error: Subclasses passed to `register` must be statically resolvable to classes or modules
    Root::B::Child => :"test/testdata/packager/track_descendants/b/b.rb",
  #                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Expected a string file path for `Root::B::Child`
    Root::B::Child => "test/testdata/packager/track_descendants/b/" + "b.rb",
  #                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Expected a string file path for `Root::B::Child`
    Root::B::Child => "test/testdata/packager/track_descendants/b/descendants.rbi",
  #                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Root::B::Child` is defined in `test/testdata/packager/track_descendants/b/b.rb`, not in `test/testdata/packager/track_descendants/b/descendants.rbi`
    # A descendant may only be registered from the package that defines it.
    Root::A::LocalChild => "test/testdata/packager/track_descendants/a/a.rb",
  # ^^^^^^^^^^^^^^^^^^^ error: `Root::A::LocalChild` must be tracked in its own package
    Root::B::Plain => "test/testdata/packager/track_descendants/b/b.rb",
  # ^^^^^^^^^^^^^^ error: `Root::B::Plain` is not a descendant of `Root::A::Parent`
  }
)

# A leading `./` is ignored.
T::Descendants.register(Root::B::Child, {Root::B::GrandChild => "./test/testdata/packager/track_descendants/b/b.rb"})
# The braces around the hash are optional.
T::Descendants.register(Root::A::Mixin, Root::B::Includer => "test/testdata/packager/track_descendants/b/b.rb")

# Each superclass may only be registered once per package.
T::Descendants.register(Root::A::Parent, {Root::B::Child => "test/testdata/packager/track_descendants/b/b.rb"}) # error: Descendants of `Root::A::Parent` have already been registered in this package

T::Descendants.register(Root::A::Parent) # error: `register` must be called with exactly 2 positional arguments
T::Descendants.register(Root::A::Parent, {Root::B::Child => "test/testdata/packager/track_descendants/b/b.rb"}, "extra") # error: `register` must be called with exactly 2 positional arguments

T::Descendants.register(Root::A::Parent, [Root::B::Child]) # error: Expected a hash literal mapping subclasses to file paths as the second argument to `register`

T::Descendants.register(Root::A::Parent.itself, {Root::B::Child => "test/testdata/packager/track_descendants/b/b.rb"})
#                       ^^^^^^^^^^^^^^^^^^^^^^ error: First argument to `register` must be statically resolvable to a class or module

T::Descendants.register(Root::A::DoesNotExist, {Root::B::Child => "test/testdata/packager/track_descendants/b/b.rb"})
#                       ^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist`
#                       ^^^^^^^^^^^^^^^^^^^^^ error: First argument to `register` must be statically resolvable to a class or module

T::Descendants.register(Root::A::NotAModule, {Root::B::Child => "test/testdata/packager/track_descendants/b/b.rb"})
#                       ^^^^^^^^^^^^^^^^^^^ error: First argument to `register` must be statically resolvable to a class or module
