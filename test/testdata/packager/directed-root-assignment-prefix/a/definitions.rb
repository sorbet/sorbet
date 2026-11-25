# typed: true

::A::X = 1 # error: Defining a root-scoped constant requires this package to be marked `prelude!`
A::Y = 2 # error: File belongs to package `A::B` but defines a constant that does not match this namespace
