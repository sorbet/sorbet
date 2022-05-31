# typed: true

class GenericPlaceholder
  extend T::Helpers
  extend T::Generic

  ElementPlaceholder = type_template {{fixed: String}}
end
