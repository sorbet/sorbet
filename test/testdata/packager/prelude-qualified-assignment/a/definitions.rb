# typed: true

A::X = 1 # error: File belongs to package `A::B` but defines a constant that does not match this namespace
     # ^ error: Expected `String` but found `Integer(1)` for field
::A::X = 'valid'
