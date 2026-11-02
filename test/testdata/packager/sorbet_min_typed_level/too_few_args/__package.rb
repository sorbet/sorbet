# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class TooFewArgs < PackageSpec
  sorbet # error: Missing required keyword argument `min_typed_level`
  strict_dependencies 'false'
  layer 'a'
end
