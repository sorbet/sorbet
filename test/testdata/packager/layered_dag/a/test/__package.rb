# frozen_string_literal: true
# typed: strict

class Test::A < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'false'

  import A
  import C
end
