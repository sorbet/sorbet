# typed: strict

class Outer::Missing < PackageSpec # error: `Outer::Missing` is nested under `Outer`, so it must `import Outer`
end
