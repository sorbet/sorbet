# typed: strict

# Importing a package that imports `Outer` isn't enough, unless that package is between `Outer` and this one.
class Outer::Sibling < PackageSpec
  import Outer::Ok
end
