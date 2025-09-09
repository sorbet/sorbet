# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class Prelude::First < PackageSpec
  prelude_package

  layer 'a'
  strict_dependencies 'layered'

  import Prelude::Second

  export Prelude::First::A
end
