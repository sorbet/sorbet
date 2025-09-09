# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility, product

class Prelude::First < PackageSpec
  prelude_package

  layer "product"
  #     ^^^^^^^^^ error: Prelude package `Prelude::First` must be in the lowest layer, `utility`
  strict_dependencies "dag"

  export Prelude::First::A
end
