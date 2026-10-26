# typed: strict
# enable-packager: true

class Rooted < PackageSpec
  export ::Rooted::Public # error: Argument to `export` must not use a `::` prefix
end
