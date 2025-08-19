# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class Block < PackageSpec
  sorbet min_typed_level: 'true', tests_min_typed_level: 'true' do end # error: Invalid expression in package: `Block` not allowed
  strict_dependencies 'false'
  layer 'a'
end
