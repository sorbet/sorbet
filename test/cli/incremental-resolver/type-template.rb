# typed: true

class A
  extend T::Generic

  # TODO(jvilk): Restore after fixing https://github.com/sorbet/sorbet/issues/1064
  # Elem = type_template(fixed: Integer)
end
