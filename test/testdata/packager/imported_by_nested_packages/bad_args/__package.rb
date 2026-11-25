# typed: strict

class BadArgs < PackageSpec
  imported_by_nested_packages!(:arg)
                             # ^^^^ error: Too many arguments
end
