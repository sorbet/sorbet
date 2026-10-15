# typed: strict
# enable-packager: true

class Rooted < PackageSpec
  export ::Rooted::Public # error: Argument to `export` must be a constant
end
