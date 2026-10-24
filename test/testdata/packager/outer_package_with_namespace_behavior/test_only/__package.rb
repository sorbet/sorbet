# typed: strict

class Outer::TestOnly < PackageSpec # error: `Outer::TestOnly` is nested under `Outer`, so it must `import Outer`
  import Outer::Ok
  test_import Outer
end
