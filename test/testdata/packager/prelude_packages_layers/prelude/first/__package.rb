# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility, product

class Prelude::First < PackageSpec
  prelude_package

  layer "product"
  strict_dependencies "dag"

  export Prelude::First::A
end
