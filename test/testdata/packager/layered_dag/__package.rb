# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility,power,business,api,service

class Root < PackageSpec
  strict_dependencies 'layered_dag'
  layer 'business'
end
