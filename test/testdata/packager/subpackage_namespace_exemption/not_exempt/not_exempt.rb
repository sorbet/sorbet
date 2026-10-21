# typed: true

module NotExempt
  extend T::Generic # error: `T.class_of(NotExempt)` is part of a subpackage namespace, and must not have ancestors
  Elem = type_member # error: `NotExempt` is part of a subpackage namespace, and must not be generic
end
