# frozen_string_literal: true
# typed: strict

class Service1 < PackageSpec
  strict_dependencies 'layered'
  layer 'service'

  import Business1
  import Utility2 # error: Strict dependencies violation: All of `Service1`'s `import`s must be `layered` or higher
end
