# typed: strict

class Outer::Nested::Other < PackageSpec # error: `Outer::Nested::Other` is nested under `Outer::Nested`, so it must `import Outer::Nested`
end
