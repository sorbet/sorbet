# typed: true

module Mid
  module Inner
    extend T::Generic # error: `T.class_of(Mid::Inner)` is part of a subpackage namespace, and must not have ancestors
    Elem = type_member # error: `Mid::Inner` is part of a subpackage namespace, and must not be generic
  end
end
