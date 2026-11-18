# typed: strict

class AppService < PackageSpec
  layer 'app_service'
  strict_dependencies 'layered'
end
