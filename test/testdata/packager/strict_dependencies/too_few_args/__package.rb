# frozen_string_literal: true
# typed: strict
# enable-packager: true

class TooFewArgs < PackageSpec
  strict_dependencies # error: Not enough arguments provided for method `PackageSpec.strict_dependencies`. Expected: `1`, got: `0`
end
