# typed: strict

class UnsatisfiedClassRoot::Sub < PackageSpec # error: `UnsatisfiedClassRoot::Sub` is nested under `UnsatisfiedClassRoot`, so it must `import UnsatisfiedClassRoot`
end
