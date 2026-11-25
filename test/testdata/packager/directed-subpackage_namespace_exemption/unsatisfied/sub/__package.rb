# typed: strict

class Unsatisfied::Sub < PackageSpec # error: `Unsatisfied::Sub` is nested under `Unsatisfied`, so it must `import Unsatisfied`
end
