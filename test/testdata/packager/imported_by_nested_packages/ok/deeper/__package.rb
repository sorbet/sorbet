# typed: strict

# Importing `Outer::Ok` is enough, because `Outer::Ok` has to import `Outer` itself.
class Outer::Ok::Deeper < PackageSpec
  import Outer::Ok
end
