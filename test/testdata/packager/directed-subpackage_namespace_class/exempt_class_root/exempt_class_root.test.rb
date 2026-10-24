# typed: true

class Test::ExemptClassRoot # error: `Test::ExemptClassRoot` cannot be a `class`, because `Test::ExemptClassRoot` encloses the tests of packages nested under `ExemptClassRoot`
end
