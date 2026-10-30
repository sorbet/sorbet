# typed: strict
# enable-packager: true

class Only::Test < PackageSpec
  test!

  export Only::Test::Thing
end
