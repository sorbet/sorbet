# typed: true

SharedAssignment = 1 # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
::SharedAssignment = 2

class ::RootBox
  NestedAssignment = 3
end
