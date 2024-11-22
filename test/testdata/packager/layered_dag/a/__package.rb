# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility,power,business,api,service

class A < PackageSpec
  strict_dependencies 'layered_dag'
  layer 'business'

  import B # error: importing `B` will put `A` into a cycle, which is not valid at `strict_dependencies` level `layered_dag`
  test_import C
end
