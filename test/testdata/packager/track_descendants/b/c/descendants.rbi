# typed: strict

T::Descendants.register(Root::A::Parent, Root::B::C::NestedChild)

# A parent package can't register descendants for its subpackages, or vice versa.
T::Descendants.register(Root::A::Parent, Root::B::Child)
#                                        ^^^^^^^^^^^^^^ error: `Root::B::Child` must be tracked in its own package
