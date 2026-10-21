# typed: strict

class UnsatisfiedBehavior::Sub < PackageSpec # error: `UnsatisfiedBehavior::Sub` is nested under `UnsatisfiedBehavior`, so it must `import UnsatisfiedBehavior`
end
