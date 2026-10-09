# typed: strict

# Synthetic String references are allowed, but do not exempt the definitions.
class String::RejectedClass # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
end
String::REJECTED_FIELD = T.let(1, Integer) # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
String::REJECTED_FIELD = T.let(2, Integer) # error: File belongs to package `Prelude` but defines a constant that does not match this namespace

# Explicit-root definitions in unpackaged namespaces are valid controls.
class ::String::ValidClass
end
::String::VALID_FIELD = T.let(3, Integer)
