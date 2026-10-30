# typed: strict

class Rooted < PackageSpec
  export_all!
  export ::Rooted::Public # error: Argument to `export` must not use a `::` prefix
end
