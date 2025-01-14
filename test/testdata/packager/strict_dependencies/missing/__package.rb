# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class Missing < PackageSpec # error: This package does not declare a `strict_dependencies` level
  layer 'a'
end
