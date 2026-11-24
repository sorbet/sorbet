# typed: true

module Root
  extend T::Generic
       # ^^^^^^^^^^ error: `T.class_of(Root)` cannot have ancestors, because packages are nested under `Root`

  Elem = type_member
       # ^^^^^^^^^^^ error: `Root` cannot be generic, because packages are nested under `Root`
end
