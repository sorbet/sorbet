# typed: strict

class Root::B < PackageSpec
  export Root::B
# ^^^^^^^^^^^^^^ error: Symbol `Root::B` is not defined in a file owned by package `Root::B`
end
