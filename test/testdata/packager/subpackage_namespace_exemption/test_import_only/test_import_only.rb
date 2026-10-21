# typed: true

module TestImportOnly
  extend T::Generic # error: `T.class_of(TestImportOnly)` is part of a subpackage namespace, and must not have ancestors
  Elem = type_member # error: `TestImportOnly` is part of a subpackage namespace, and must not be generic
end
