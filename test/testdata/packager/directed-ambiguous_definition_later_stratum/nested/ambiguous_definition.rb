# typed: true

# Defining `module A::B` requires Ruby to look up `A` at runtime. Without an
# autoload for `Nested::A`, that lookup can find the root-level `::A`, while
# Sorbet places the definition at `Nested::A::B`. A directory-based autoloader
# may ensure that `Nested::A` exists first, but Sorbet is loader-agnostic and
# conservatively reports 5068 "Definition of `B` is ambiguous" in monolithic
# mode when the alternate lexical `A` is visible.
#
# In package-directed mode `A` is in a later stratum than this file, so `::A`
# has not been entered yet when `findAnyDefinitionAmbiguousWithCurrent` runs,
# but the resolver consults the package registry to discover that it might
# exist at runtime.
module Nested
  module A::B # error: Definition of `B` is possibly ambiguous
  end
end
