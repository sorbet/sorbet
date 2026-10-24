# typed: strict

class TestImportOnly::Sub < PackageSpec # error: `TestImportOnly::Sub` is nested under `TestImportOnly`, so it must `import TestImportOnly`
  test_import TestImportOnly
end
