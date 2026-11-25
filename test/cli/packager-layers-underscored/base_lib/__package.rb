# typed: strict

class BaseLib < PackageSpec
  layer 'base_lib'
  strict_dependencies 'layered'

  import AppService
end
