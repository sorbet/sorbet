# typed: true
class EnumProp
  include T::Props
  prop :enum_field, T.deprecated_enum(["hi", "there"])
end

EnumProp.new.enum_field
