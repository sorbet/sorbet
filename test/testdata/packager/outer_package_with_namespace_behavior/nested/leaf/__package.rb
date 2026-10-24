# typed: strict

class Outer::Nested::Leaf < PackageSpec # error: `Outer::Nested::Leaf` is nested under `Outer::Nested`, so it must `import Outer::Nested`
  import Outer
end
