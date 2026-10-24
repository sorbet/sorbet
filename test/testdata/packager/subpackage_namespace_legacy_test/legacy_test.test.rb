# typed: true

module Test::LegacyTest
  extend T::Generic # error: `T.class_of(Test::LegacyTest)` cannot have ancestors, because `Test::LegacyTest` encloses the tests of packages nested under `LegacyTest`
  Elem = type_member # error: `Test::LegacyTest` cannot be generic, because `Test::LegacyTest` encloses the tests of packages nested under `LegacyTest`
end
