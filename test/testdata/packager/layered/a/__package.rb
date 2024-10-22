# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility,power,business,api,service

class A < PackageSpec
  strict_dependencies 'layered'
  layer 'business'

  import B # error: `A` is at layer `business`, so it can not import package `B`, which is at layer `service`
  import C
end
