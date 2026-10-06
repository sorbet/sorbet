# typed: true

# `Opus::Foo::Bar` imports `Opus::Foo`, so opening `module Opus::Foo` here is allowed. But `Opus::Foo::Baz` is a
# sibling subpackage that imports *us*, so it lives in a later stratum than this file. When this file is
# resolved in package-directed mode, the `Opus::Foo::Baz` namespace has not been entered yet.
#
# Ruby resolves the bare `Baz` below to `Opus::Foo::Baz` (via the `Opus::Foo` cref), and so does Sorbet in
# monolithic mode, reporting "`Opus::Foo::Baz` is not imported" and giving `X` the type `T.untyped`.
#
# In package-directed mode the lexical lookup currently misses at `Opus::Foo`, climbs to the root scope, and
# silently binds `Baz` to the prelude's `::Baz`. Assert the monolithic behavior that directed mode should match.
module Opus::Foo
  module Bar
    X = Baz # error: `Opus::Foo::Baz` resolves but is not imported
    T.reveal_type(X) # error: Revealed type: `T.untyped`
  end
end
