# typed: strict

# The error is reported on `Outer::Missing` instead, which is what has to change.
class Outer::Missing::Deeper < PackageSpec
  import Outer::Missing
end
