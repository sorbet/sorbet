# typed: strict
# stratum: 1

# Importing `Dep` places `Parent` in a later stratum than `Parent::Test`, which
# does not import `Parent`.
class Parent < PackageSpec
  import Dep

  export Parent::Thing
  export Parent::Inner::Other
end
