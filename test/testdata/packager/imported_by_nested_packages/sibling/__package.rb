# typed: strict

# Importing a package that imports `Outer` isn't enough, unless that package is between `Outer` and this one.
class Outer::Sibling < PackageSpec # error: `Outer::Sibling` is nested under `Outer`, so it must `import Outer`
  import Outer::Ok
end
