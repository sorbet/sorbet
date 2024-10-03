# frozen_string_literal: true
# typed: strict
# enable-packager: true

class TooManyArgs < PackageSpec
  strict_dependencies 'false', 'true' # error: Too many arguments provided for method `PackageSpec.strict_dependencies`. Expected: `1`, got: `2`
end
