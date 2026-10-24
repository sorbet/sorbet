# typed: strict

class BadArgs < PackageSpec
  outer_package_with_namespace_behavior!(:arg)
                                       # ^^^^ error: Too many arguments
end
