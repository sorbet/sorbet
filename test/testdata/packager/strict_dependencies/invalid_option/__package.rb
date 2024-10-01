# frozen_string_literal: true
# typed: strict
# enable-packager: true

class InvalidOption < PackageSpec
  strict_dependencies 'true' # error: Argument to `strict_dependencies` must be one of: `'false'`, `'layered'`, `'layered_dag'` or `'dag'`
end
