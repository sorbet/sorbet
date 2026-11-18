# typed: strict

T::Descendants.register(Root::A::Parent, Root::B::C::NestedChild, "test/testdata/packager/track_descendants/b/c/c.rb")

# A parent package can't register descendants for its subpackages, or vice versa.
T::Descendants.register(Root::A::Parent, Root::B::Child, "test/testdata/packager/track_descendants/b/b.rb")
#                                        ^^^^^^^^^^^^^^ error: `Root::B::Child` must be tracked in its own package
