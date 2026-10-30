# typed: strict

class Target::TestOnlyThing # error: Tests in the `Target` package must define tests in the `Test::Target` namespace
end
