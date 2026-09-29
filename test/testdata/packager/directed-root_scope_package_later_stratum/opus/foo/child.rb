# typed: true

# Sorbet resolves a bare constant by walking the lexical scopes (including root) and only then the ancestors of the
# innermost scope. In monolithic mode `Baz` therefore finds the top-level package namespace `::Baz` first: 3718
# "`Baz` is not imported", type `T.untyped`.
#
# In package-directed mode package `Baz` is a later stratum than this file, so `::Baz` has not been entered; the
# root lookup misses and the ancestor lookup finds `Base::Baz` instead, with no error. (Ruby itself checks
# ancestors before `Object`, so Ruby agrees with the directed result here -- but the two Sorbet modes disagree with
# each other.) Assert the monolithic behavior that directed mode should match.
class Opus::Foo::Child < Base
  X = Baz # error: `Baz` resolves but is not imported
  T.reveal_type(X) # error: Revealed type: `T.untyped`
end
