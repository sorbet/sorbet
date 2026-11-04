# typed: true

module MidBad
  module Inner
    extend T::Generic # error: `T.class_of(MidBad::Inner)` cannot have ancestors, because packages are nested under `MidBad::Inner`
    Elem = type_member # error: `MidBad::Inner` cannot be generic, because packages are nested under `MidBad::Inner`
  end
end
