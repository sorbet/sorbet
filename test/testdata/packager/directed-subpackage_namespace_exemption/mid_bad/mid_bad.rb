# typed: true

module MidBad
  module Inner
    extend T::Generic # error: `T.class_of(MidBad::Inner)` is part of a subpackage namespace, and must not have ancestors
    Elem = type_member # error: `MidBad::Inner` is part of a subpackage namespace, and must not be generic
  end
end
