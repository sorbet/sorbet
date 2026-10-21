# typed: true

module WithTestPackage
  extend T::Generic # error: `T.class_of(WithTestPackage)` is part of a subpackage namespace, and must not have ancestors
  Elem = type_member # error: `WithTestPackage` is part of a subpackage namespace, and must not be generic
end
