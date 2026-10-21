# typed: true

module Deep
  extend T::Generic # error: `T.class_of(Deep)` is part of a subpackage namespace, and must not have ancestors
  Elem = type_member # error: `Deep` is part of a subpackage namespace, and must not be generic
end
