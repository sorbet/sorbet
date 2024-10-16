# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class ValidOption < PackageSpec
  strict_dependencies 'false'
  layer 'a'
end
