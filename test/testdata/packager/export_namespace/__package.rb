# typed: strict
# enable-packager: true

class Root < PackageSpec
  export Root
# ^^^^^^^^^^^ error: Symbol `Root` is not defined in a file owned by package `Root`
end
