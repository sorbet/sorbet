# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class Block < PackageSpec
  strict_dependencies 'false' do end
  #                           ^^^^^^ error: Invalid expression in package: `Block` not allowed
  #                           ^^^^^^ error: does not take a block
  layer 'a'
end
