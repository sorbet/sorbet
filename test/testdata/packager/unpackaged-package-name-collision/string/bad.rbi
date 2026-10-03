# typed: true

class String # error: File belongs to package `String` but defines a constant that does not match this namespace
  class ::Consumer::FromRBI < ::Object # error: Superclasses may only be set on constants in the package that owns them
  end
end
