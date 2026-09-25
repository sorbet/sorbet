# typed: true

# `Opus::Foo::Bar` imports `Opus::Foo`, so opening `module Opus::Foo` here is allowed. But `Opus::Foo::Baz` is a
# sibling subpackage that imports *us*, so it lives in a later stratum than this file. When this file is
# resolved in package-directed mode, the `Opus::Foo::Baz` namespace has not been entered yet.
#
# Ruby resolves the bare `Baz` below to `Opus::Foo::Baz` (via the `Opus::Foo` cref), and so does Sorbet in
# monolithic mode, reporting "`Opus::Foo::Baz` is not imported" and giving `X` the type `T.untyped`.
#
# Package-directed mode must report the same thing, rather than letting the lexical lookup miss at `Opus::Foo`,
# climb to the root scope, and silently bind `Baz` to the prelude's `::Baz`.
module Opus::Foo
  module Bar
    X = Baz
      # ^^^ error: `Opus::Foo::Baz` is not imported
    T.reveal_type(X) # error: Revealed type: `T.untyped`
  end
end
