# typed: strict

# Root-scoping a constant is allowed in a non-prelude package when the constant
# still matches the package namespace.
class ::Root::Nested::AlsoAllowed
end

::Root::Nested::AlsoAllowedAssignment = 1
