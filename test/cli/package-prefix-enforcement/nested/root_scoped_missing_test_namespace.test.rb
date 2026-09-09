# typed: strict

# The constant matches the package path, but tests in this package must also use
# the Test:: namespace. Root-scoping does not bypass that requirement.
class ::Root::Nested::MissingTestNamespace
end
