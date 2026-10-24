# typed: strict

class Outer::Nested < PackageSpec
  outer_package_with_namespace_behavior!

  import Outer
end
