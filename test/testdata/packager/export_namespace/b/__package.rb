# typed: strict

class Root::B < PackageSpec
  export Root::B
# ^^^^^^^^^^^^^^ error: Constant `Root::B` lacks a declaration in this package and cannot be exported
end
