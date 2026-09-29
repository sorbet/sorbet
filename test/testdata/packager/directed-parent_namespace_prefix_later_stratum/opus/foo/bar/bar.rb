# typed: true

# `Opus::Foo::Baz` is not a package: only `Opus::Foo::Baz::Qux` is. In monolithic mode the namer has already
# created the `Opus::Foo::Baz` namespace (from `module Opus::Foo::Baz::Qux`), it is attributed to `Opus::Foo`
# (which we import), and `Baz` below resolves to it: 3717 "`Opus::Foo::Baz` resolves but is not exported from
# `Opus::Foo`", type `T.class_of(Opus::Foo::Baz)`.
#
# In package-directed mode `Opus::Foo::Baz::Qux` is a later stratum than this file (it imports us), so the
# `Opus::Foo::Baz` namespace does not exist yet. The lexical lookup misses at `Opus::Foo`, climbs to root, and
# binds to the prelude's `::Baz` with no error. Importing `Opus::Foo` does not help, and neither would stopping
# the walk at package namespaces (`Opus::Foo::Baz` is only a registry prefix, with no owning package). This
# snapshot records the current package-directed behavior.
module Opus::Foo
  module Bar
    X = Baz
    T.reveal_type(X) # error: Revealed type: `T.class_of(Baz)`
  end
end
