# typed: strict
# enable-packager: true

# stratum: 2

class Consumer < PackageSpec
  # Importing a child package means the `Provider` namespace already exists
  # when this package is resolved, even though `Provider` itself hasn't been.
  import Provider::Child
end
