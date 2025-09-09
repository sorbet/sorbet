# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Prelude::Second < PackageSpec
  prelude_package

  layer "utility"
  strict_dependencies "dag"

  import Prelude::First

  export Prelude::Second::B
end
