# typed: strict

class Outer::Mid::Leaf < PackageSpec # error: `Outer::Mid::Leaf` is nested under `Outer`, so it must `import Outer`
end
