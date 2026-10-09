# typed: true

class String # error: File belongs to package `String` but defines a constant that does not match this namespace
  class ::Consumer::FromRBI < ::Object # error: Defining a root-scoped constant requires this package to be marked `prelude!`
  #                           ^^^^^^^^ error: Superclasses may only be set on constants in the package that owns them
  end
end
