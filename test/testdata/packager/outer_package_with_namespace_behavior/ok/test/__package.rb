# typed: strict

# `test!` packages are not special: like any other package, importing the package that encloses them is enough.
class Outer::Ok::Test < PackageSpec
  test!

  import Outer::Ok, uses_internals: true
end
