# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility,power,business,api,service

class B < PackageSpec
  strict_dependencies 'layered'
  layer 'business'

  import A
end
