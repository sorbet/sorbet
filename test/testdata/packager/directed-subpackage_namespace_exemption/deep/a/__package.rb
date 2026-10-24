# typed: strict

class Deep::A < PackageSpec
  outer_package_with_namespace_behavior!

  import Deep
end
