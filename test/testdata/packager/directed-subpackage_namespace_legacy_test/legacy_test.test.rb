# typed: true

module Test::LegacyTest
  extend T::Generic # error: `T.class_of(Test::LegacyTest)` is part of a subpackage namespace, and must not have ancestors
  Elem = type_member # error: `Test::LegacyTest` is part of a subpackage namespace, and must not be generic
end
