# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Block < PackageSpec
  strict_dependencies 'false' do end # error: Invalid expression in package: `Block` not allowed
end
