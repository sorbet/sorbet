# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility,power,business,api,service

class C < PackageSpec
  strict_dependencies 'layered_dag'
  layer 'business'

  import A
end
