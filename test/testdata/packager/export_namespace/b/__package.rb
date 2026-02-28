# typed: strict

class Root::B < PackageSpec
  export Root::B
# ^^^^^^^^^^^^^^ error: Constant `Root::B` lacks a declaration and cannot be exported
end
