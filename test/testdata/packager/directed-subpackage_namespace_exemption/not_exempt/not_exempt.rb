# typed: true

module NotExempt
  extend T::Generic # error: `T.class_of(NotExempt)` cannot have ancestors, because packages are nested under `NotExempt`
  Elem = type_member # error: `NotExempt` cannot be generic, because packages are nested under `NotExempt`
end
