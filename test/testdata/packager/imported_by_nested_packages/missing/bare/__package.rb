# typed: strict

class Outer::Missing::Bare < PackageSpec # error: `Outer::Missing::Bare` is nested under `Outer`, so it must `import Outer`
end
