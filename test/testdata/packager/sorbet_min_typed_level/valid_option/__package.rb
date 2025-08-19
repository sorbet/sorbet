# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class ValidOption < PackageSpec
  sorbet min_typed_level: 'true', tests_min_typed_level: 'true'
  strict_dependencies 'false'
  layer 'a'
end
