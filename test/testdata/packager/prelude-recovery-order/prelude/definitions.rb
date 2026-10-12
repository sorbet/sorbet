# typed: true

# Later rejected reopenings must not redirect an earlier definition's scope.
class String::Early # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
end
class String # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
end

Array::EARLY = 1 # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
class Array # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
end

class ::LocalRoot; end
class LocalRoot::Early # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
end
class LocalRoot # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
end

# Ownership checks inside recovery scopes must not recommend adding prelude!.
class LocalRoot::Generic # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
  extend T::Generic
  Elem = type_member
end

::String::Early
::Array::EARLY
::LocalRoot::Early
::LocalRoot::Generic
