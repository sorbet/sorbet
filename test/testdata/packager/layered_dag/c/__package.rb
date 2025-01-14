# frozen_string_literal: true
# typed: strict

class C < PackageSpec
  strict_dependencies 'layered_dag'
  layer 'business'

  import A
end
