# typed: strict

class Deep::A < PackageSpec
  imported_by_nested_packages!

  import Deep
end
