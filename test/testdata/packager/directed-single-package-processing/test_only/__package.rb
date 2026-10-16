# typed: strict

class TestOnly < PackageSpec
  export TestOnly::Missing
  #      ^^^^^^^^ error: Unable to resolve constant `TestOnly`
  #      ^^^^^^^^ error: Unable to resolve constant `TestOnly`
  export Test::TestOnly::Public
end
