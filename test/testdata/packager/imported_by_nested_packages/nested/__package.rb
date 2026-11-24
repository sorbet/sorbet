# typed: strict

class Outer::Nested < PackageSpec
  imported_by_nested_packages!

  import Outer
end
