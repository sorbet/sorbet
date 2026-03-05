# typed: strict
# enable-packager: true

class Root::A < PackageSpec
  export Root::A
# ^^^^^^^^^^^^^^ error: Constant `Root::A` lacks a declaration and cannot be exported
end
