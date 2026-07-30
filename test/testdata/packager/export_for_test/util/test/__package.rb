# typed: strict

class Test::Opus::Util < PackageSpec
  test!

  import Opus::Util, uses_internals: true

  export Test::Opus::Util::TestUtil
end
