# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class Repeated < PackageSpec
  sorbet min_typed_level: 'true', tests_min_typed_level: 'true'
  sorbet min_typed_level: 'true', tests_min_typed_level: 'true' # error: Repeated declaration of `sorbet`
  strict_dependencies 'false'
  layer 'a'
end
