# typed: true

# In Ruby, the `A` in `module A::B` is looked up lexically from `Opus`, and would find the root-level `::A` defined
# by package `A`. Sorbet always defines `Opus::A::B` here and instead reports 5068 "Definition of `B` is ambiguous"
# when a lexically-visible `A` exists -- in monolithic mode this file gets exactly that error.
#
# In package-directed mode `A` is a later stratum than this file, so `::A` has not been entered yet when
# `findAnyDefinitionAmbiguousWithCurrent` runs, and nothing is reported. The symbol is placed the same way in
# both modes; only the error differs. This snapshot records the current package-directed behavior.
module Opus
  module A::B
  end
end
