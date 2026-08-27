# frozen_string_literal: true
# typed: strict

class Prelude::Second < PackageSpec
  prelude!

  layer 'a'
  strict_dependencies 'layered'

  import Prelude::First

  export Prelude::Second::B
end
