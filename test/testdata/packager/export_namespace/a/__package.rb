# typed: strict
# enable-packager: true

class Root::A < PackageSpec
  export Root::A
# ^^^^^^^^^^^^^^ error: Symbol `Root` is not defined in a file owned by package `Root`
end
