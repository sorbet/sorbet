# typed: strict

class Test::Opus::TestImported < PackageSpec
  test!

  import Opus::TestImported

  export Test::Opus::TestImported::TITestClass
end
