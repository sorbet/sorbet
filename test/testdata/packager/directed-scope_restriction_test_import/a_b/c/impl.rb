# typed: true

# `A::B::C` only `test_import`s `A::B`. A test import is not usable from application code, so it should not make it
# legal for this (application) file to open `module A::B`. It also does not order `A::B` before this file: `A::B`
# imports `A::B::C`, so in package-directed mode `A::B` is processed *after* this file, and `A::B::Secret` does not
# exist yet when `Secret` below is resolved.
#
# Ruby resolves `Secret` to `A::B::Secret` (via the `A::B` cref). Sorbet's lexical lookup misses at `A::B`, climbs
# to the root scope, and binds to the prelude's `::Secret` instead: a different constant, with no error to say so.
#
# Today `canOpenScope` treats the `test_import` like an `import`, so no 5086 is reported on `module A::B` either.
# This snapshot records that behavior; the next commit changes it.
module A::B
  module C
    X = Secret
    T.reveal_type(X) # error: Revealed type: `T.class_of(Secret)`
  end
end
