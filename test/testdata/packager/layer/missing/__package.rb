# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class Missing < PackageSpec # error: This package does not declare a `layer`
  strict_dependencies 'false'
end
