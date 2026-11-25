# typed: true

class String # error: File belongs to package `String` but defines a constant that does not match this namespace
  class ::Consumer::FromRBI < ::Object
#       ^^^^^^^^^^ error: Defining a root-scoped constant requires this package to be marked `prelude!`
  end
end
