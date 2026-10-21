# typed: true

module Deep::A
  extend T::Generic # error: `T.class_of(Deep::A)` is part of a subpackage namespace, and must not have ancestors
  Elem = type_member # error: `Deep::A` is part of a subpackage namespace, and must not be generic
end
