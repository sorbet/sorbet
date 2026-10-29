# typed: true

module Root
  extend T::Generic
       # ^^^^^^^^^^ error: `T.class_of(Root)` is part of a subpackage namespace, and must not have ancestors

  Elem = type_member
       # ^^^^^^^^^^^ error: `Root` is part of a subpackage namespace, and must not be generic
end
