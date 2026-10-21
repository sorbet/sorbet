# typed: true

module Exempt
  extend T::Generic # error: `T.class_of(Exempt)` is part of a subpackage namespace, and must not have ancestors
  Elem = type_member # error: `Exempt` is part of a subpackage namespace, and must not be generic
end
